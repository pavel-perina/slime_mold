//! \file ui.h

#pragma once

#include <memory>

class Ui final
{
public:

    //! \brief Constructor
    Ui();

    //! \brief Destructor
    ~Ui();

    //! \brief Render next frame
    void frame();

    //! \brief Check if done (closed window)
    [[nodiscard]] bool done() const noexcept;

    //! \brief Returns true if class were initialized.
    [[nodiscard]] bool initialized() const noexcept;

private:
    static constexpr int SIMULATION_WIDTH  = 640;
    static constexpr int SIMULATION_HEIGHT = 480;
    static constexpr int SIDEPANEL_WIDTH   = 224;
    static constexpr int TOTAL_WIDTH = SIDEPANEL_WIDTH + SIMULATION_WIDTH;
    class Private;
    std::unique_ptr<Private> m_p;
};
