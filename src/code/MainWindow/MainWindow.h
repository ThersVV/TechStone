#pragma once
#include "TextThingy.h"

#include <fstream>
#include <gtkmm-4.0/gtkmm.h>

/**
 * @class MainWindow
 * @brief The main windows of the application
 *
 * Inherits from Gtk::Window.
 */
class MainWindow : public Gtk::Window
{
public:
    /**
     * @brief Constructor for MainWindow.
     *
     * Calls the second contructor with empty source file path and empty starting text
     */
    MainWindow();
    /**
     * @brief Constructor for MainWindow.
     *
     * @param _sourceFilePath The path to the file the program should save the contents. Empty if no path is set yet.
     * @param startingText The text the program should start with
     */
    MainWindow(std::string _sourceFilePath, std::string startingText);

private:
    enum class PopupType
    {
        Error,
        Success
    };
    void setKeyHandler();
    void setup_grid();
    void on_save_file(Glib::RefPtr<Gio::AsyncResult> &result, Glib::RefPtr<Gtk::FileDialog> dialog);
    void save_to(std::string filePath);
    bool on_key_pressed(guint keyval, guint keycode, Gdk::ModifierType state);
    void show_save_error(const std::string message);
    void show_save_success(const std::string message);
    void show_popup(const std::string message, const PopupType type);
    void setup_overlay();
    void setup_css();
    bool beforeClose();
    void trueClose();

    TextThingy textArea;
    Gtk::Overlay overlay;
    Gtk::Box verticalBox;
    Gtk::Box horizontalBox;
    Gtk::Grid panelGrid;

    std::string _sourceFilePath;

    const int SIDEPANEL_WIDTH_FRACTION = 14;
    const int SIDEPANEL_HEIGHT_FRACTION = 5;
    const int DEFAULT_WIDTH = 1080;
    const int DEFAULT_HEIGHT = 500;
};
