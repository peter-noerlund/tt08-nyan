#include "window.h"

#include <iostream>
#include <fstream>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QSizePolicy>
#include <QGraphicsView>
#include <QPixmap>

#include <cstddef>

namespace tt06
{

Window::Window(QWidget* parent)
    : QWidget(parent)
{
    buildUI();

    m_timer.setSingleShot(false);
    connect(&m_timer, &QTimer::timeout, this, &Window::refreshMonitor);
    m_timer.start(std::chrono::milliseconds(25));
}

void Window::buildUI()
{
    auto verticalLayout = new QVBoxLayout(this);

    // Graphicsview

    auto horizontalLayout = new QHBoxLayout();
    horizontalLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    
    m_monitor = new Monitor(this);

    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(m_monitor->sizePolicy().hasHeightForWidth());
    m_monitor->setSizePolicy(sizePolicy);
    m_monitor->setMinimumSize(QSize(640, 480));
    m_monitor->setMaximumSize(QSize(640, 480));

    horizontalLayout->addWidget(m_monitor);

    horizontalLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

    verticalLayout->addLayout(horizontalLayout);

    // Buttons

    auto gridLayout = new QGridLayout();
    gridLayout->setContentsMargins(0, 0, 0, 0);

    for (std::size_t i = 0; i != m_inputButtons.size(); ++i)
    {
        auto input = new QCheckBox(this);
        gridLayout->addWidget(input, i, 0, 1, 1);
        connect(input, &QCheckBox::stateChanged, [this, i](int state)
        {
            onInputChanged(i, state);
        });
        m_inputButtons[i] = input;
    }

    for (std::size_t i = 0; i != m_bidirButtons.size(); ++i)
    {
        auto input = new QCheckBox(this);
        gridLayout->addWidget(input, i, 1, 1, 1);
        connect(input, &QCheckBox::stateChanged, [this, i](int state)
        {
            onBidirChanged(i, state);
        });
        m_bidirButtons[i] = input;
    }

    m_enableButton = new QCheckBox(this);
    gridLayout->addWidget(m_enableButton, m_inputButtons.size(), 0, 1, 1);
    connect(m_enableButton, &QCheckBox::stateChanged, this, &Window::onEnableChanged);

    m_resetButton = new QCheckBox(this);
    gridLayout->addWidget(m_resetButton, m_bidirButtons.size(), 1, 1, 1);
    connect(m_resetButton, &QCheckBox::stateChanged, this, &Window::onResetChanged);

    verticalLayout->addLayout(gridLayout);

    retranslateUI();

    m_simulator = std::make_unique<Simulator>(m_monitor);
    m_simulator->run();
}

void Window::retranslateUI()
{
    setWindowTitle(tr("TinyTapeout 8 Contest Simulator"));
    
    for (std::size_t i = 0; i != m_inputButtons.size(); ++i)
    {
        m_inputButtons[i]->setText(tr("Input %1").arg(i));
    }
    for (std::size_t i = 0; i != m_bidirButtons.size(); ++i)
    {
        m_bidirButtons[i]->setText(tr("Bidirectional %1").arg(i));
    }
    m_enableButton->setText("Enable");
    m_resetButton->setText("Reset (active low)");
}

void Window::onResetChanged(int state)
{
    m_simulator->setValue(Simulator::Input::Reset, state == Qt::CheckState::Checked);
}

void Window::onEnableChanged(int state)
{
    m_simulator->setValue(Simulator::Input::Enable, state == Qt::CheckState::Checked);
}

void Window::onInputChanged(std::size_t idx, int state)
{
    m_simulator->setValue(static_cast<Simulator::Input>(static_cast<unsigned int>(Simulator::Input::Input0) + idx), state == Qt::CheckState::Checked);
}

void Window::onBidirChanged(std::size_t idx, int state)
{
    m_simulator->setValue(static_cast<Simulator::Input>(static_cast<unsigned int>(Simulator::Input::Bidir0) + idx), state == Qt::CheckState::Checked);
}

void Window::refreshMonitor()
{
    m_monitor->update();
}


} // namespace tt06