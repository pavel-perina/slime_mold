//! \file ui.h

#pragma once

#include <memory>

class Ui
{
public:

    //! \brief Constructor
    Ui();

    //! \brief Destructor
    virtual ~Ui();

    //! \brief Render next frame
    void frame();

    //! \brief Check if done (closed window)
    bool done();

    //! \brief Returns true if class were initialized.
    bool initialized();

private:
    static constexpr int SIDEPANEL_WIDTH = 224;
    class Private;
    std::unique_ptr<Private> m_p;
};
