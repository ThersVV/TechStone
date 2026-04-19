#include "MainWindow.h"
#include "TopPanel.h"

#include <iostream>

MainWindow::MainWindow() : MainWindow("", "")
{
}

MainWindow::MainWindow(std::string sourceFilePath, std::string startingText) : _sourceFilePath(sourceFilePath), textArea(startingText)
{
    set_title("Tech Stone");
    setup_css();

    set_default_size(DEFAULT_WIDTH, DEFAULT_HEIGHT); // todo change to fullscreen windowed

    setup_overlay();
    setup_grid();

    setKeyHandler();
    signal_close_request().connect(sigc::mem_fun(*this, &MainWindow::beforeClose), false);
}

void MainWindow::setup_grid()
{
    verticalBox.set_orientation(Gtk::Orientation::VERTICAL);
    horizontalBox.set_orientation(Gtk::Orientation::HORIZONTAL);
    verticalBox.set_vexpand(true);
    horizontalBox.set_vexpand(true);
    verticalBox.set_hexpand(true);
    horizontalBox.set_hexpand(true);

    const int SIDEPANEL_WIDTH = DEFAULT_WIDTH / SIDEPANEL_WIDTH_FRACTION;
    const int TOPPANEL_HEIGHT = DEFAULT_HEIGHT / SIDEPANEL_HEIGHT_FRACTION;

    Gtk::Frame *emptyTop = Gtk::make_managed<TopPanel>(textArea.pageInterface);
    emptyTop->set_size_request(-1, TOPPANEL_HEIGHT);
    verticalBox.append(*emptyTop);

    Gtk::Frame *emptyLeft = Gtk::make_managed<Gtk::Frame>();
    emptyLeft->set_size_request(SIDEPANEL_WIDTH, -1);
    emptyLeft->set_vexpand(true);
    horizontalBox.append(*emptyLeft);

    Gtk::Box *emptyLeftResizing = Gtk::make_managed<Gtk::Box>();
    emptyLeftResizing->set_hexpand(true);
    emptyLeftResizing->set_vexpand(true);
    horizontalBox.append(*emptyLeftResizing);

    horizontalBox.append(textArea);

    Gtk::Box *emptyRightResizing = Gtk::make_managed<Gtk::Box>();
    emptyRightResizing->set_hexpand(true);
    emptyRightResizing->set_vexpand(true);
    horizontalBox.append(*emptyRightResizing);

    Gtk::Frame *emptyRight = Gtk::make_managed<Gtk::Frame>();
    emptyRight->set_size_request(SIDEPANEL_WIDTH, -1);
    emptyRight->set_vexpand(true);
    horizontalBox.append(*emptyRight);

    verticalBox.append(horizontalBox);
}

void MainWindow::trueClose()
{
    // I want to pretend I've saved
    textArea.onSave();
    close();
}

bool MainWindow::beforeClose()
{
    if (textArea.getEdited())
    {
        // todo: add some space between the buttons
        auto closeDialog = Gtk::make_managed<Gtk::MessageDialog>(
            *this,
            "Are you sure you want to exit without saving?",
            false, Gtk::MessageType::QUESTION, Gtk::ButtonsType::YES_NO);

        closeDialog->set_transient_for(*this);

        closeDialog->signal_response().connect(
            [this, closeDialog](int response)
            {
                response == Gtk::ResponseType::YES ? trueClose() : closeDialog->destroy();
            });

        closeDialog->set_visible();
        return true;
    }
    return false;
}

void MainWindow::setKeyHandler()
{
    auto keyController = Gtk::EventControllerKey::create();
    keyController->signal_key_pressed().connect(sigc::mem_fun(*this, &MainWindow::on_key_pressed), false);
    this->add_controller(keyController);
}

void MainWindow::on_save_file(Glib::RefPtr<Gio::AsyncResult> &result, Glib::RefPtr<Gtk::FileDialog> dialog)
{
    try
    {
        auto filePath = dialog->save_finish(result)->get_path();
        save_to(filePath);
    }
    catch (Gtk::DialogError _e)
    {
        show_save_error("An error occured while saving the file");
    }
}

void MainWindow::save_to(std::string filePath)
{
    // todo: Stupid implementation, maybe save page by page so we dont eat ALL the RAM.
    auto data = textArea.getAllText();

    std::ofstream fileStream(filePath, std::ios::out | std::ios::trunc);
    if (!fileStream)
    {
        show_save_error("Saving failed, could not open file " + filePath);
    }

    std::ostreambuf_iterator<char> out_it(fileStream);
    std::copy(data.begin(), data.end(), out_it);
    _sourceFilePath = filePath;
    show_save_success("File saved successfully");
    textArea.onSave();
}

bool MainWindow::on_key_pressed(guint keyval, guint keycode, Gdk::ModifierType state)
{
    if (keyval == GDK_KEY_s && (state & Gdk::ModifierType::CONTROL_MASK) == Gdk::ModifierType::CONTROL_MASK)
    {
        if (_sourceFilePath == "")
        {
            auto dialog = Gtk::FileDialog::create();
            dialog->save(*this, sigc::bind(sigc::mem_fun(*this, &MainWindow::on_save_file), dialog));
        }
        else
        {
            save_to(_sourceFilePath);
        }
        return true;
    }
    return false;
}

void MainWindow::show_save_error(const std::string message)
{
    show_popup(message, PopupType::Error);
}

void MainWindow::show_save_success(const std::string message)
{
    show_popup(message, PopupType::Success);
}

void MainWindow::show_popup(const std::string message, const MainWindow::PopupType type)
{
    // Multiple popups can be shown at the same time, but I don't think I care :d
    auto label = Gtk::make_managed<Gtk::Label>(message);
    label->set_margin(10);

    auto frame = Gtk::make_managed<Gtk::Frame>();
    frame->set_margin_end(20);
    frame->set_margin_bottom(20);
    frame->set_halign(Gtk::Align::END);
    frame->set_valign(Gtk::Align::END);
    frame->set_size_request(200, 50);

    frame->set_child(*label);
    if (type == PopupType::Error)
    {
        frame->set_css_classes({"popup", "error-popup"});
    }
    else if (type == PopupType::Success)
    {
        frame->set_css_classes({"popup", "success-popup"});
    }

    overlay.add_overlay(*frame);

    Glib::signal_timeout().connect_seconds_once(
        [this, frame]()
        { overlay.remove_overlay(*frame); },
        2);
}

void MainWindow::setup_overlay()
{
    overlay.set_halign(Gtk::Align::FILL);
    overlay.set_valign(Gtk::Align::FILL);
    overlay.set_hexpand(true);
    overlay.set_vexpand(true);

    overlay.set_child(verticalBox);
    set_child(overlay);
}

void MainWindow::setup_css()
{
    auto css_provider = Gtk::CssProvider::create();
    css_provider->load_from_path("code/MainWindow/MainWindow.css");
    auto display = Gdk::Display::get_default();
    Gtk::StyleContext::add_provider_for_display(
        display, css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}