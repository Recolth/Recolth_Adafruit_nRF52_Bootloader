#!/bin/bash

source ~/myenv/bin/activate && compiledb make BOARD=xiao_nrf52840_ble_sense all && make BOARD=xiao_nrf52840_ble_sense copy-artifact && make BOARD=xiao_nrf52840_ble_sense clean