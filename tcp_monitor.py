import ctypes
import time
import threading

import uvicorn
from fastapi import FastAPI
from bcc import BPF
from socket import AF_INET
from socket import inet_ntop
from struct import pack


class TrafficMonitor(object):
    def __init__(self, interval):
        self.metrics = {}
        self.interval = interval

        code = open('tcp_monitor.c').read()
        self.program = BPF(text=code)
        self.program.attach_kprobe(event='tcp_sendmsg', fn_name='poll_sendmsg')
        self.program.attach_kprobe(event='tcp_recvmsg', fn_name='poll_recvmsg')

    def poll_metrics_from_bpf_program(self):
        ts_key = ctypes.c_uint32(0)
        hashmap_dict_key_pair = {
            'send': self.program['send_bytes'],
            'recv': self.program['recv_bytes']
        }

        while True:
            try:
                time.sleep(self.interval)
                kernel_time = self.program['ktime_map'][ts_key].value
                current_time = time.time_ns()
                ts_offset = current_time - kernel_time
                current_metrics = {
                    'send': {},
                    'recv': {}
                }

                for direction, bpf_hash in hashmap_dict_key_pair.items():
                    for entry, value in bpf_hash.items():
                        ts = value.timestamp + ts_offset
                        if current_time - ts > 60_000_000_000:
                            bpf_hash.pop(entry)
                        else:
                            src_ip = inet_ntop(
                                AF_INET, pack('I', entry.src_ip))
                            dst_ip = inet_ntop(
                                AF_INET, pack('I', entry.dst_ip))
                            key = (
                                f'{src_ip}:{entry.src_port}'
                                f'->{dst_ip}:{entry.dst_port}'
                            )

                            current_metrics[direction][key] = value.bytes
                self.metrics = current_metrics

            except KeyboardInterrupt:
                print('Terminating...')
                return

    def start(self):
        thread = threading.Thread(
            target=self.poll_metrics_from_bpf_program,
            daemon=True
        )
        thread.start()

        return thread


POLL_INTERVAL = 1
app = FastAPI()
monitor = TrafficMonitor(interval=POLL_INTERVAL)


@app.get('/metrics')
def get_metrics():
    return monitor.metrics


def start():
    monitor.start()
    uvicorn.run(app, host='0.0.0.0', port=8000)


if __name__ == '__main__':
    start()
