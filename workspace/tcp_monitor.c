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
};

struct ip_leaf_t {
    u64 bytes;
    u64 timestamp;
};

BPF_HASH(send_bytes, struct ip_key_t, struct ip_leaf_t, MAX_ENTRIES);
BPF_HASH(recv_bytes, struct ip_key_t, struct ip_leaf_t, MAX_ENTRIES);

// kernel time map for calculating the time drift with python monitor app
BPF_HASH(ktime_map, u32, u64, 1);

static int update_metrics(
    struct pt_regs *ctx,
    struct sock *sk,
    size_t size,
    int direction
) {
    if (sk == NULL)
        return 0;

    u32 src_ip = 0, dst_ip = 0;
    u16 src_port = 0, dst_port = 0;
    struct ip_key_t ip_key = {};
    struct ip_leaf_t *ip_leaf, new_leaf = {};

    bpf_probe_read(&src_ip, sizeof(src_ip), &sk->__sk_common.skc_rcv_saddr);
    bpf_probe_read(&dst_ip, sizeof(dst_ip), &sk->__sk_common.skc_daddr);
    bpf_probe_read(&src_port, sizeof(src_port), &sk->__sk_common.skc_num);
    bpf_probe_read(&dst_port, sizeof(dst_port), &sk->__sk_common.skc_dport);

    ip_key.src_ip = src_ip;
    ip_key.dst_ip = dst_ip;
    ip_key.src_port = src_port;
    ip_key.dst_port = bpf_ntohs(dst_port);

    u32 key = 0;
    u64 ts = bpf_ktime_get_ns();
    ktime_map.update(&key, &ts);

    if (direction == 0) {
        ip_leaf = send_bytes.lookup(&ip_key);
        if (ip_leaf) {
            lock_xadd(&ip_leaf->bytes, size);
            ip_leaf->timestamp = ts;
        } else {
            new_leaf.bytes = size;
            new_leaf.timestamp = ts;
            send_bytes.update(&ip_key, &new_leaf);
        }
    } else {
        ip_leaf = recv_bytes.lookup(&ip_key);
        if (ip_leaf) {
            lock_xadd(&ip_leaf->bytes, size);
            ip_leaf->timestamp = ts;
        } else {
            new_leaf.bytes = size;
            new_leaf.timestamp = ts;
            recv_bytes.update(&ip_key, &new_leaf);
        }
    }

    return 0;
}

int poll_sendmsg(
    struct pt_regs *ctx,
    struct sock *sk,
    struct msghdr *msg,
    size_t size
) {
    return update_metrics(ctx, sk, size, 0);
}

int poll_recvmsg(
    struct pt_regs *ctx,
    struct sock *sk,
    struct msghdr *msg,
    int flags,
    size_t len
) {
    return update_metrics(ctx, sk, len, 1);
}
