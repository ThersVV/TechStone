#include "TopPanel.h"

TopPanel::TopPanel(std::shared_ptr<PageInterface> pageInterface_) : pageInterface(pageInterface_)
{
    initButtonBox();
    set_child(buttonBox);
}

void TopPanel::initButtonBox()
{
    initButtons();
    buttonBox.set_hexpand();
}

void TopPanel::initButtons()
{
    auto redButton = Gtk::make_managed<StylingButton>("red");
    redButton->set_label("RED");
    addBasicStylingButton(redButton);

    auto boldButton = Gtk::make_managed<StylingButton>("bold");
    boldButton->set_label("BOLD");
    addBasicStylingButton(boldButton);

    auto italicsButton = Gtk::make_managed<StylingButton>("italics");
    italicsButton->set_label("ITALICS");
    addBasicStylingButton(italicsButton);

    auto underlineButton = Gtk::make_managed<StylingButton>("underlined");
    underlineButton->set_label("UNDERLINED");
    addBasicStylingButton(underlineButton);
}

void TopPanel::addBasicStylingButton(StylingButton *button)
{
    button->set_margin(5);
    button->set_focusable(false);
    auto tagName = button->GetTagName();
    button->signal_clicked().connect(
        [this, tagName]()
        {
            pageInterface->TagSelectedText(tagName);
        });
    buttonBox.append(*button);
}