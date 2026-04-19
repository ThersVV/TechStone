#include "TextThingy.h"

TextThingy::TextThingy() : TextThingy("")
{
}

TextThingy::TextThingy(std::string startingText)
{
    pageInterface = std::make_shared<PageInterface>(*this, startingText);
    set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
    set_vexpand(true);
    set_hexpand(false);
    set_size_request(pageWidth, 200);

    initPages();
    addFixedOverlay();
    dropHandler();
    this->get_vscrollbar()->get_adjustment()->signal_value_changed().connect(sigc::mem_fun(*this, &TextThingy::on_scroll), true);
    edited = false;
}

void TextThingy::onSave()
{
    edited = false;
}

bool TextThingy::getEdited()
{
    return edited;
}

void TextThingy::setEdited()
{
    edited = true;
}

void TextThingy::on_scroll()
{
    auto adjustment = this->get_vscrollbar()->get_adjustment();
    auto pageToUpdate = vScrollToPage(adjustment);
}

std::size_t TextThingy::vScrollToPage(Glib::RefPtr<Gtk::Adjustment> adjustment)
{
    double top_visible_y = adjustment->get_value();
    double visible_height = adjustment->get_page_size();
    double bottom_visible_y = top_visible_y + visible_height;
    return (bottom_visible_y / (pageDeviderHeight + pageHeight)) + 1;
}

void TextThingy::addFixedOverlay()
{
    overlay = Gtk::Fixed();
    overlay.set_size_request(pageWidth, -1);
    overlay.set_vexpand(true);
    overlay.set_hexpand(true);
    pages.set_hexpand(true);
    pages.set_vexpand(true);

    overlay.put(pages, 0, 0);
    set_child(overlay);
}

void TextThingy::initPages()
{
    pages.set_orientation(Gtk::Orientation::VERTICAL);
    pages.set_size_request(pageWidth, -1);
    pages.set_hexpand(true);

    addPage();
}

void TextThingy::addPage()
{
    // This assumes that pageInterface already added a page
    pages.append(*pageInterface->GetLastPage());

    Gtk::Box *emptySpace = Gtk::make_managed<Gtk::Box>();
    emptySpace->set_hexpand(true);
    emptySpace->set_vexpand(true);

    emptySpace->set_size_request(-1, pageDeviderHeight);
    pages.append(*emptySpace);
}

void TextThingy::removePage(std::size_t index)
{
    auto children = pages.get_children();
    pages.remove(*children[index * 2 + 1]);
    pages.remove(*children[index * 2]);
}

void TextThingy::removeLastPage()
{
    pages.remove(*pages.get_last_child());
    pages.remove(*pages.get_last_child());
}

std::string TextThingy::getAllText()
{
    std::string result;
    auto approximatelyMedianPageLen = pageInterface->GetPageContents(pageInterface->GetPageCount() / 2).size();
    result.reserve((pageInterface->GetPageCount() + 1) * approximatelyMedianPageLen);

    for (std::size_t i = 0; i < pageInterface->GetPageCount(); i++)
    {
        result.append(pageInterface->GetPageContents(i));
    }
    return result;
}

void TextThingy::printFile(gpointer data, gpointer user_data)
{
    GFile *file = G_FILE(data);
    std::cout << "Item: " << g_file_get_basename(file) << std::endl;
    std::cout << "Item path: " << g_file_get_path(file) << std::endl;
}

void TextThingy::staticTryImportImage(gpointer data, gpointer user_data)
{
    CoordinatesStatic *self = static_cast<CoordinatesStatic *>(user_data);
    if (self)
    {
        self->self->tryImportImage(std::forward<gpointer>(data), &(self->coords));
    }
}

void TextThingy::tryImportImage(gpointer data, gpointer user_data)
{
    GFile *file = G_FILE(data);
    auto [x, y] = *(Coordinates *)(user_data);

    auto image = Gtk::make_managed<Gtk::Picture>(g_file_get_path(file));
    image->set_opacity(0.8);
    image->set_content_fit(Gtk::ContentFit::FILL);
    auto pixbuf = Gdk::Pixbuf::create_from_file(g_file_get_path(file));

    image->set_size_request(pixbuf->get_width(), pixbuf->get_height());

    //  I can put some guards to only put it to valid coordinates, but that is not necessary I think
    overlay.put(*image, x, y);
}

bool TextThingy::handleFileDrop(const Glib::ValueBase &value, double x, double y)
{
    CoordinatesStatic coords = CoordinatesStatic(x, y, this);
    if (G_VALUE_HOLDS(value.gobj(), gdk_file_list_get_type()))
    {
        // It's a file (file list to be more exact)
        GdkFileList *file_list = static_cast<GdkFileList *>(g_value_get_boxed(value.gobj()));
        auto files = gdk_file_list_get_files(file_list);
        // g_slist_foreach(files, printFilename, nullptr);
        g_slist_foreach(files, TextThingy::staticTryImportImage, &coords);

        return true;
    }
    else
    {
        std::cout << "Received unexpected data type: \"" << G_VALUE_TYPE_NAME(value.gobj()) << std::endl;
        return false;
    }
}

void TextThingy::dropHandler()
{
    auto dnd = Gtk::DropTarget::create(GDK_TYPE_FILE_LIST, Gdk::DragAction::COPY);

    dnd->signal_drop().connect(sigc::mem_fun(*this, &TextThingy::handleFileDrop), false);
    add_controller(dnd);
}
