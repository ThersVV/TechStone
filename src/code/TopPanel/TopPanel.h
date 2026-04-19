#pragma once

#include "TextThingy.h"
#include <gtkmm-4.0/gtkmm.h>

/**
 * @class TopPanel
 * @brief Vertical panel with buttons
 *
 * Currently represents specically the top menu of the UI
 */
class TopPanel : public Gtk::Frame
{
public:
    /**
     * @brief Constructor for TopPanel
     *
     * @param pageInterface_ Pointer to the PageInterface that is to be interacted with by the menu buttons
     */
    TopPanel(std::shared_ptr<PageInterface> pageInterface_);

private:
    class StylingButton : public Gtk::Button
    {
    public:
        StylingButton(std::string stylingTagName_) : stylingTagName(stylingTagName_) {}
        std::string GetTagName()
        {
            return stylingTagName;
        }

    private:
        std::string stylingTagName;
    };
    void initButtonBox();
    void initButtons();
    void addBasicStylingButton(StylingButton *button);

    Gtk::Box buttonBox;
    // todo: probably a dict that matches enum to buttons?
    Gtk::Button redButton;

    std::shared_ptr<PageInterface> pageInterface;
};