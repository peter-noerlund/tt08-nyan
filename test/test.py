# SPDX-FileCopyrightText: © 2024 Tiny Tapeout
# SPDX-License-Identifier: Apache-2.0

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import ClockCycles
from cocotb.triggers import RisingEdge
from cocotb.triggers import FallingEdge


def get_vsync(dut):
    return dut.uo_out[3].value


def get_hsync(dut):
    return dut.uo_out[7].value


def get_pwm(dut):
    return dut.uio_out[7].value


async def get_sample(dut):
    value = 0
    for clk in range(0, 128):
        value += get_pwm(dut)
        await ClockCycles(dut.clk, 1)
    return value


async def test_audio(dut):
    dut._log.info("[Audio] Verify I/O")

    assert dut.uio_oe[7]

    dut._log.info("[Audio] Wait for next sample")

    value = get_pwm(dut)
    assert value

    for clk in range(0, 128):
        await ClockCycles(dut.clk, 1)
        value = get_pwm(dut)
        if not value:
            break

    for clk in range(0, 128):
        await ClockCycles(dut.clk, 1)
        value = get_pwm(dut)
        if value:
            break

    assert value

    dut._log.info("[Audio] Verify initial samples")

    for i in range(1, 4):
        for j in range(0, 6):
            value = await get_sample(dut)
            assert value == i


async def test_vga(dut):
    h_pixels = 640
    h_front_porch = 16
    h_sync_pulse = 96
    h_back_porch = 48
    h_frame = h_pixels + h_front_porch + h_sync_pulse + h_back_porch

    v_pixel = 480
    v_front_porch = 10
    v_sync_pulse = 2
    v_back_porch = 33
    v_frame = v_pixel + v_front_porch + v_sync_pulse + v_back_porch

    dut._log.info("[Video] Wait for horizontal sync")

    hsync = get_hsync(dut)
    assert hsync

    for clk in range(0, h_back_porch + h_pixels + h_front_porch):
        await ClockCycles(dut.clk, 1)
        hsync = get_hsync(dut)
        if not hsync:
            break
    assert not hsync

    dut._log.info("[Video] Verify horizontal sync pulse length")

    for clk in range(0, h_sync_pulse):
        await ClockCycles(dut.clk, 1)
        hsync = get_hsync(dut)
        if hsync:
            break
    assert hsync
    assert clk == h_sync_pulse - 1

    dut._log.info("[Video] Verify horizontal timing")

    for clk in range(0, h_back_porch + h_pixels + h_front_porch):
        await ClockCycles(dut.clk, 1)
        hsync = get_hsync(dut)
        if not hsync:
            break

    assert not hsync
    assert clk == h_back_porch + h_pixels + h_front_porch - 1

    dut._log.info("[Video] Wait for vertical sync")
    
    vsync = get_vsync(dut)
    assert vsync

    for clk in range(0, h_frame * v_frame):
        await ClockCycles(dut.clk, 1)
        vsync = get_vsync(dut)
        if not vsync:
            break
    assert not vsync

    dut._log.info("[Video] Verify vertical sync pulse length")
    for clk in range(0, h_frame * v_sync_pulse):
        await ClockCycles(dut.clk, 1)
        vsync = get_vsync(dut)
        if vsync:
            break
    assert vsync
    assert clk == h_frame * v_sync_pulse - 1

    dut._log.info("[Video] Verify line count")

    hsync = get_hsync(dut)
    assert hsync

    lines = 0
    for clk in range(0, h_frame * (v_back_porch + v_pixel + v_front_porch)):
        old_hsync = hsync
        await ClockCycles(dut.clk, 1)
        hsync = get_hsync(dut)

        if old_hsync and not hsync:
            lines = lines + 1

        vsync = get_vsync(dut)
        if not vsync:
            break

    assert lines == v_back_porch + v_pixel + v_front_porch
    assert not vsync
    assert clk == h_frame * (v_back_porch + v_pixel + v_front_porch) - 1




@cocotb.test()
async def test_project(dut):
    dut._log.info("Start")

    clock = Clock(dut.clk, 40, units="ns")
    cocotb.start_soon(clock.start())

    # Reset
    dut._log.info("Reset")
    dut.ena.value = 1
    dut.ui_in.value = 0
    dut.uio_in.value = 0
    dut.rst_n.value = 0
    await ClockCycles(dut.clk, 10)
    dut.rst_n.value = 1

    dut._log.info("Test project behavior")

    await ClockCycles(dut.clk, 1)

    #await test_audio(dut)
    await test_vga(dut)
