#include "MainWindow.h"
#include <fstream>
#include <gtkmm-4.0/gtkmm.h>
#include <iostream>

std::string readFile(std::string filename)
{
    if (filename == "")
    {
        return "";
    }

    std::ifstream file(filename);
    if (!file)
    {
        throw std::runtime_error("Filename " + filename + " could not be opened properly!");
    }
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

std::string getFilename(std::vector<std::string> &args)
{
    if (args.size() > 2)
    {
        throw std::runtime_error("You cannot open more than one file at once!");
    }

    if (args.size() == 2)
    {
        return args[1];
    }
    return "";
}

void makeWindow(Glib::RefPtr<Gtk::Application> app, std::string filename)
{
    // todo: check for leaks
    auto startingText = readFile(filename);
    auto window = new MainWindow(filename, startingText);
    app->add_window(*window);
    window->show();
}

int main(int argc, char *argv[])
{
    std::vector<std::string> args(argv, argv + argc);

    auto app = Gtk::Application::create("org.gtkmm.example");
    app->signal_activate().connect(sigc::bind(sigc::ptr_fun(&makeWindow), app, getFilename(args)));

    return app->run();
}
