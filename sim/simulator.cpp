#include "simulator.h"

#include "monitor.h"

#include <thread>

namespace tt08
{

Simulator::Simulator(Monitor* monitor)
    : m_context(std::make_shared<Context>())
{
    m_context->top.clk = 0;
    m_context->top.ui_in = 0;
    m_context->top.uio_in = 0;
    m_context->top.ena = 0;
    m_context->top.clk = 0;
    m_context->top.rst_n = 0;

    m_context->monitor = monitor;
}

Simulator::~Simulator()
{
    m_context->stopPending.store(true, std::memory_order_release);

    std::unique_lock<std::mutex> lock(m_context->mutex);
    m_context->stopCondition.wait(lock);
}

void Simulator::setValue(Input input, bool value)
{
    auto val = static_cast<unsigned int>(input);

    if (value)
    {
        m_context->nextInputs.store(m_context->nextInputs.load(std::memory_order_relaxed) | (1 << val), std::memory_order_release);
    }
    else
    {
        m_context->nextInputs.store(m_context->nextInputs.load(std::memory_order_relaxed) & ~(1 << val), std::memory_order_release);
    }
}

void Simulator::run()
{
    auto context = m_context;
    std::thread thread([context]()
    {
        while (step(*context))
        {
        }
        finish(*context);
    });
    thread.detach();
}

bool Simulator::step(Context& context)
{
    if (context.stopPending.load(std::memory_order_acquire))
    {
        return false;
    }

    auto inputs = context.nextInputs.load(std::memory_order_acquire);
    context.top.ui_in = static_cast<std::uint8_t>(inputs >> static_cast<unsigned int>(Input::Input0));
    context.top.uio_in = static_cast<std::uint8_t>(inputs >> static_cast<unsigned int>(Input::Bidir0));
    context.top.ena = (inputs & (1 << static_cast<unsigned int>(Input::Enable))) ? 1 : 0;
    context.top.rst_n = (inputs & (1 << static_cast<unsigned int>(Input::Reset))) ? 1 : 0;
    context.top.clk ^= 1;

    context.top.eval();
    if (context.top.clk == 1)
    {
        updateMonitor(context);
    }

    return true;
}

void Simulator::updateMonitor(Context& context)
{
    // PMod signal      HS B0 G0 R0 VS B1 G1 R1

    auto pmod = context.top.uo_out;
    bool hsync = (pmod & 0x80) != 0;
    bool vsync = (pmod & 0x08) != 0;

    if (!hsync && context.column >= s_horizontalBackPorch && context.column < s_width + s_horizontalBackPorch &&
        !vsync && context.row >= s_verticalBackPorch && context.row < s_height + s_verticalBackPorch)
    {
        // Encoding is RRRGGGBB

        auto x = context.column - s_horizontalBackPorch;
        auto y = context.row - s_verticalBackPorch;

        auto red = (((pmod & 0x01) << 1) | ((pmod & 0x10) >> 4)) * 85;
        auto green = (((pmod & 0x02) << 0) | ((pmod & 0x20) >> 5)) * 85;
        auto blue = (((pmod & 0x04) >> 1) | ((pmod & 0x40) >> 6)) * 85;

        context.monitor->setPixel(x, y, red, green, blue);
    }

    if (!hsync)
    {
        ++context.column;
    }
    if (context.oldHsync && !hsync)
    {
        context.column = 0;
        ++context.row;
    }
    if (context.oldVsync && !vsync)
    {
        context.row = 0;
    }

    context.oldHsync = hsync;
    context.oldVsync = vsync;
}

void Simulator::finish(Context& context)
{
    context.top.final();

    std::lock_guard<std::mutex> lock(context.mutex);
    context.stopCondition.notify_one();
}

} // namespace tt08