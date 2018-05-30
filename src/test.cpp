#include <iostream>
#include <thread>
#include "RecursiveDirectoryMonitor.h"

int main() {
    Gio::init();
    bool end = true;

    FileTools::RecursiveDirectoryMonitor recDir(Gio::File::create_for_path("src"), [&](const Glib::RefPtr<Gio::File>& o, const Glib::RefPtr<Gio::File>& n, Gio::FileMonitorEvent e){
	end = false;		    
    });

    while(end) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
	return 0;
}
