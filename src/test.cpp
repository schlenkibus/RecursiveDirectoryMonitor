#include <iostream>
#include <thread>
#include "RecursiveDirectoryMonitor.h"

int main() {
    Gio::init();

    bool end = true;
    auto myCallBack = [&](const Glib::RefPtr<Gio::File>& oldFile,const Glib::RefPtr<Gio::File>& newFile,Gio::FileMonitorEvent monitorEvent) {
        std::cout << "changed: "  << oldFile->get_path();
        end = false;
    };

    FileTools::RecursiveDirectoryMonitor<decltype(myCallBack)> recDir(Gio::File::create_for_path("src"), myCallBack);

    while(end) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
	return 0;
}
