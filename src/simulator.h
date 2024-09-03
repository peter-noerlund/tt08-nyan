#pragma once

#include "Vtop.h"

#include <atomic>
#include <cstdint>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <span>
#include <vector>

class Vtop;

namespace tt06
{

class Monitor;

class Simulator
{
public:
    enum class Input
    {
        Input0,
        Input1,
        Input2,
        Input3,
        Input4,
        Input5,
        Input6,
        Input7,
        Bidir0,
        Bidir1,
        Bidir2,
        Bidir3,
        Bidir4,
        Bidir5,
        Bidir6,
        Bidir7,
        Enable,
        Reset
    };
    
    struct Output
    {
        std::uint8_t output = 0;
        std::uint8_t bidirOutput = 0;
    };

    explicit Simulator(Monitor* monitor);
    ~Simulator();

    inline void setHigh(Input input)
    {
        setValue(input, true);
    }

    inline void setLow(Input input)
    {
        setValue(input, false);
    }
    
    void setValue(Input input, bool value);
    void run();

private:
    static constexpr unsigned int s_width = 640;
    static constexpr unsigned int s_horizontalFrontPorch = 16;
    static constexpr unsigned int s_horizontalSyncPulse = 96;
    static constexpr unsigned int s_horizontalBackPorch = 48;

    static constexpr unsigned int s_height = 480;
    static constexpr unsigned int s_verticalFrontPorch = 10;
    static constexpr unsigned int s_verticalSyncPulse = 2;
    static constexpr unsigned int s_verticalBackPorch = 33;

    struct Context
    {
        Vtop top;
        bool oldVsync = false;
        bool oldHsync = false;
        unsigned int row = 0;
        unsigned int column = 0;
        Monitor* monitor = nullptr;

        std::array<std::uint8_t, 64> filler1;

        std::atomic_bool stopPending = false;
        std::atomic_uint32_t nextInputs = 0;
        
        std::array<std::uint8_t, 64> filler2;

        std::mutex mutex;
        std::condition_variable stopCondition;
        
    };

    static bool step(Context& context);
    static void finish(Context& context);
    static void updateMonitor(Context& context);

    std::shared_ptr<Context> m_context;
};
    
} // namespace tt06