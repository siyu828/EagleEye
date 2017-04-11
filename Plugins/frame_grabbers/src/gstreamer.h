#pragma once

#include "cv_capture.h"
#include "Aquila/ICoordinateManager.h"
#include "RuntimeLinkLibrary.h"
#include "frame_grabbersExport.hpp"

namespace aq
{
    namespace Nodes
    {
        class frame_grabbers_EXPORT GrabberGstreamer: public GrabberCV
        {
        public:
            MO_DERIVE(GrabberGstreamer, GrabberCV)
                PARAM(bool, loop, true);
                MO_SIGNAL(void, eof);
            MO_END;

            bool Load(const std::string& file_path);
            
            static int CanLoad(const std::string& document);
            static void ListPaths(std::vector<std::string>& paths);
        };
    }
}
