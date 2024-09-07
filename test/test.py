# SPDX-FileCopyrightText: © 2024 Tiny Tapeout
# SPDX-License-Identifier: Apache-2.0

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import ClockCycles
from cocotb.triggers import RisingEdge
from cocotb.triggers import FallingEdge


def get_vsync(pmod):
    return pmod[3].value


def get_hsync(pmod):
    return pmod[7].value

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

    dut._log.info("[VGA] Wait for horizontal sync")

    hsync = get_hsync(dut.uo_out)
    assert hsync

    for clk in range(0, h_back_porch + h_pixels + h_front_porch):
        await ClockCycles(dut.clk, 1)
        hsync = get_hsync(dut.uo_out)
        if not hsync:
            break
    assert not hsync

    dut._log.info("[VGA] Verify horizontal sync pulse length")

    for clk in range(0, h_sync_pulse):
        await ClockCycles(dut.clk, 1)
        hsync = get_hsync(dut.uo_out)
        if hsync:
            break
    assert hsync
    assert clk == h_sync_pulse - 1

    dut._log.info("[VGA] Verify horizontal timing")

    for clk in range(0, h_back_porch + h_pixels + h_front_porch):
        await ClockCycles(dut.clk, 1)
        hsync = get_hsync(dut.uo_out)
        if not hsync:
            break

    assert not hsync
    assert clk == h_back_porch + h_pixels + h_front_porch - 1

    dut._log.info("[VGA] Wait for vertical sync")
    
    vsync = get_vsync(dut.uo_out)
    assert vsync

    for clk in range(0, h_frame * v_frame):
        await ClockCycles(dut.clk, 1)
        vsync = get_vsync(dut.uo_out)
        if not vsync:
            break
    assert not vsync

    dut._log.info("[VGA] Verify vertical sync pulse length")
    for clk in range(0, h_frame * v_sync_pulse):
        await ClockCycles(dut.clk, 1)
        vsync = get_vsync(dut.uo_out)
        if vsync:
            break
    assert vsync
    assert clk == h_frame * v_sync_pulse - 1

    dut._log.info("[VGA] Verify line count")

    hsync = get_hsync(dut.uo_out)
    assert hsync

    lines = 0
    for clk in range(0, h_frame * (v_back_porch + v_pixel + v_front_porch)):
        old_hsync = hsync
        await ClockCycles(dut.clk, 1)
        hsync = get_hsync(dut.uo_out)

        if old_hsync and not hsync:
            lines = lines + 1

        vsync = get_vsync(dut.uo_out)
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

    # uio_out[7] = Audio PWM
    # uo_out[0] = R1
    # uo_out[1] = G1
    # uo_out[2] = B1
    # uo_out[3] = VSync
    # uo_out[4] = R0
    # uo_out[5] = G0
    # uo_out[6] = B0
    # uo_out[7] = HSync

    await ClockCycles(dut.clk, 2)

    assert dut.uio_oe.value == 0x80

    await test_vga(dut)
