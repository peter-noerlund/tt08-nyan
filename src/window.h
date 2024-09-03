#pragma once

#include "simulator.h"

#include "monitor.h"

#include <QWidget>
#include <QLabel>
#include <QCheckBox>
#include <QTimer>

#include <array>
#include <memory>

namespace tt06
{

class Window : public QWidget
{
    Q_OBJECT

public:
    explicit Window(QWidget* parent = nullptr);

private Q_SLOTS:
    void onResetChanged(int state);
    void onEnableChanged(int state);
    void refreshMonitor();

private:
    void buildUI();
    void retranslateUI();
    void onInputChanged(std::size_t idx, int state);
    void onBidirChanged(std::size_t idx, int state);

    Monitor* m_monitor = nullptr;
    std::array<QCheckBox*, 8> m_inputButtons = {};
    std::array<QCheckBox*, 8> m_bidirButtons = {};
    QCheckBox* m_enableButton = nullptr;
    QCheckBox* m_resetButton = nullptr;

    std::unique_ptr<Simulator> m_simulator;
    QTimer m_timer;
};

} // namespace tt06