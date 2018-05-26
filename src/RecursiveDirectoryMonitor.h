#pragma once
#include <gio/gio.h>
#include <map>
#include <giomm/file.h>
#include <giomm/init.h>
#include <iostream>

namespace FileTools {
	template<class T>
class RecursiveDirectoryMonitor : public sigc::trackable {
		protected:
			T m_callBack;
			Glib::RefPtr<Gio::File> m_rootFolder;
			std::vector<Glib::RefPtr<Gio::FileMonitor>> m_monitors;
		public:
			RecursiveDirectoryMonitor(Glib::RefPtr<Gio::File>&& rootFolder, T callback) : m_rootFolder(std::move(rootFolder)),
																								 m_callBack(callback) {
				rebuildDirectoryMap();
			}

			void rebuildDirectoryMap() {
				m_monitors.clear();
                addMonitor(m_rootFolder);
				addSubfolder(m_rootFolder);
				std::cout << "dirmap is that big: " << m_monitors.size();
			}

			void addSubfolder(const Glib::RefPtr<Gio::File>& folder) {
                try {
                    auto fileIt = folder->enumerate_children();
                    while (auto fileInfo = fileIt->next_file()) {
                        auto file = getFileFromFileInfo(folder, fileInfo);
                        switch (fileInfo->get_file_type()) {
                            case Gio::FILE_TYPE_DIRECTORY:
                                addMonitor(file);
                                addSubfolder(file);
                                break;
                            default:
                                addMonitor(file);
                                break;

                        }
                    }
                }
                catch (const Glib::Error &e) {
                    std::cerr << e.what();
                } catch(...) {
                    std::cerr << "unknown exception catched!\n";
                }
            }

			void addMonitor(const Glib::RefPtr<Gio::File>& file) {
                auto monitor = file->monitor(Gio::FILE_MONITOR_WATCH_MOUNTS);
                monitor->signal_changed().connect(sigc::mem_fun(this, &RecursiveDirectoryMonitor::onFileChanged));
				m_monitors.emplace_back(monitor);
			}

			void onFileChanged(const Glib::RefPtr<Gio::File>& oldFile,const Glib::RefPtr<Gio::File>& newFile,Gio::FileMonitorEvent monitorEvent) {
				m_callBack(oldFile, newFile, monitorEvent);
                rebuildDirectoryMap();
			}

	protected:
		Glib::RefPtr<Gio::File> getFileFromFileInfo(const Glib::RefPtr<Gio::File>& currentFolder, const Glib::RefPtr<Gio::FileInfo>& fileInfo) {
			auto name = fileInfo->get_name();
			auto path = currentFolder->get_path() + '/' + name;
			return Gio::File::create_for_path(path);
		}
	};
}
