#!/usr/bin/env bash
set -euo pipefail

sudo apt-get update
sudo apt-get install -y build-essential cmake g++ libboost-uuid-dev libnotify-bin
