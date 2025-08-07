#pragma once

#include <memory>

class Ui {
public:
    Ui();
    virtual ~Ui();
    void frame();
    bool done();
private:
    static constexpr int SIDEPANEL_WIDTH = 224;
    class Private;
    std::unique_ptr<Private> m_p;
};
