#!/usr/bin/env bash
set -euo pipefail

sudo apt-get update
sudo apt-get install -y build-essential cmake g++ libboost-all-dev libnotify-bin
sudo apt install libzstd-dev -y
sudo apt install -y libnghttp2-dev
