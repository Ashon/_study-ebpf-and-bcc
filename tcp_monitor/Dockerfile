FROM python:3.11.0-slim

WORKDIR /app

ENV PYTHONPATH="/usr/lib/python3/dist-packages"
ENV PYTHONUNBUFFERED=1

RUN apt update \
  && apt install --yes \
    bpfcc-tools \
    linux-headers-generic \
  && apt autoremove --yes --allow-remove-essential \
  && apt clean

RUN pip install bcc pytest fastapi uvicorn

COPY tcp_monitor.c tcp_monitor.py ./
