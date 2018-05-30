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
        RecursiveDirectoryMonitor(const Glib::RefPtr<Gio::File>& rootFolder, T callback) : m_rootFolder(rootFolder),
                                                                                           m_callBack(callback),
                                                                                           m_monitors(0)
        {
          rebuildDirectoryMap();
        }

        void rebuildDirectoryMap()
        {
          m_monitors.clear();
          addMonitor(m_rootFolder);
          for(auto& dir: getAllDirectorysInDirectory(m_rootFolder))
          {
            addMonitor(dir);
          }
        }

        void addMonitor(const Glib::RefPtr<Gio::File>& file)
        {
          auto monitor = file->monitor(Gio::FILE_MONITOR_WATCH_MOUNTS);
          monitor->signal_changed().connect(sigc::mem_fun(this, &RecursiveDirectoryMonitor::onFileChanged));
          m_monitors.emplace_back(std::move(monitor));
        }

        void onFileChanged(const Glib::RefPtr<Gio::File>& oldFile,const Glib::RefPtr<Gio::File>& newFile,Gio::FileMonitorEvent monitorEvent)
        {
          m_callBack(oldFile, newFile, monitorEvent);
          rebuildDirectoryMap();
        }

        std::list<Glib::RefPtr<Gio::File>> getAllFilesBeingMonitored()
        {
          return getAllFilesInFolder(m_rootFolder);
        }

        std::list<Glib::RefPtr<Gio::File>> getAllDirectorysInDirectory(const Glib::RefPtr<Gio::File>& folder)
        {
          std::list<Glib::RefPtr<Gio::File>> ret;
          auto fileIt = folder->enumerate_children();
          while (auto fileInfo = fileIt->next_file())
          {
            auto file = getFileFromFileInfo(folder, fileInfo);
            if (fileInfo->get_file_type() == Gio::FILE_TYPE_DIRECTORY)
            {
              ret.emplace_back(file);
              for(auto& f: getAllFilesInFolder(file))
              {
                ret.emplace_back(f);
              }
            }
          }
          return ret;
        }

        std::list<Glib::RefPtr<Gio::File>> getAllFilesInFolder(Glib::RefPtr<Gio::File> folder)
        {
          std::list<Glib::RefPtr<Gio::File>> ret;
          auto fileIt = folder->enumerate_children();
          while (auto fileInfo = fileIt->next_file()) {
            auto file = getFileFromFileInfo(folder, fileInfo);
            if (fileInfo->get_file_type()  == Gio::FILE_TYPE_DIRECTORY)
            {
              for(auto f: getAllFilesInFolder(file))
              {
                ret.emplace_back(f);
              }
            }
            else
            {
              ret.emplace_back(file);
            }
          }
          return ret;
        }
    protected:
        Glib::RefPtr<Gio::File> getFileFromFileInfo(const Glib::RefPtr<Gio::File>& currentFolder, const Glib::RefPtr<Gio::FileInfo>& fileInfo)
        {
          auto name = fileInfo->get_name();
          auto path = currentFolder->get_path() + '/' + name;
          return Gio::File::create_for_path(path);
        }
    };
}
