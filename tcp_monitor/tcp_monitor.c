#include <uapi/linux/ptrace.h>
#include <net/sock.h>
#include <bcc/proto.h>
#include <linux/sched.h>

// max tcp connection entries (equal to conntrack max entries)
#define MAX_ENTRIES 300000

struct ip_key_t {
    u32 src_ip;
    u32 dst_ip;
    u16 src_port;
    u16 dst_port;
    u8 is_recv;
} __attribute__((aligned(8)));

struct ip_value_t {
    u64 bytes;
    u64 timestamp;
} __attribute__((aligned(8)));

// https://elixir.bootlin.com/linux/v4.6/source/include/net/sock.h#L148
struct __sock_common {
    union {
        __addrpair skc_addrpair;
        struct {
            __be32 skc_daddr;
            __be32 skc_rcv_saddr;
        };
    };
    union  {
        unsigned int skc_hash;
        __u16 skc_u16hashes[2];
    };
    /* skc_dport && skc_num must be grouped as well */
    union {
        __portpair skc_portpair;
        struct {
            __be16 skc_dport;
            __u16 skc_num;
        };
    };
} __attribute__((preserve_access_index));

BPF_HASH(traffic_bytes, struct ip_key_t, struct ip_value_t, MAX_ENTRIES);

// kernel time map for calculating the time drift with python monitor app
BPF_HASH(ktime_map, u32, u64, 1);

static __always_inline int update_metrics(
    struct pt_regs *ctx,

    // https://elixir.bootlin.com/linux/v4.6/source/include/net/sock.h#L306
    struct sock *sk,
    size_t size,
    bool is_recv
) {
    if (sk == NULL)
        return 0;

    struct ip_key_t key = {};

    struct __sock_common sk_data = {};

    bpf_probe_read(&sk_data, sizeof(sk_data), &sk->__sk_common);

    key.src_ip = sk_data.skc_rcv_saddr;
    key.dst_ip = sk_data.skc_daddr;
    key.src_port = sk_data.skc_num;
    key.dst_port = bpf_ntohs(sk_data.skc_dport);
    key.is_recv = is_recv;

    u32 zero = 0;
    u64 ts = bpf_ktime_get_ns();
    ktime_map.update(&zero, &ts);

    struct ip_value_t *ip_value = traffic_bytes.lookup(&key);

    if (ip_value) {
        lock_xadd(&ip_value->bytes, size);
        ip_value->timestamp = ts;
    } else {
        struct ip_value_t new_leaf = {
            .bytes = size,
            .timestamp = ts
        };
        traffic_bytes.update(&key, &new_leaf);
    }

    return 0;
}

int poll_sendmsg(
    struct pt_regs *ctx,
    struct sock *sk,
    struct msghdr *msg,
    size_t size
) {
    return update_metrics(ctx, sk, size, false);
}

int poll_recvmsg(
    struct pt_regs *ctx,
    struct sock *sk,
    struct msghdr *msg,
    int flags,
    size_t len
) {
    return update_metrics(ctx, sk, len, true);
}
