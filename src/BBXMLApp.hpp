#pragma once

#include "cinder/Xml.h"
#include <string>

using namespace ci;
using namespace std;
using namespace fs;

#define MAXPATH 128
#define MAXCLIPS 10000

class BBXML {
  public:
	enum process_type {light,dark,gray,spot};
	enum stage {left,center,right,any};
	enum content {child,teen,adult};
	enum angle {section,axial};

	struct CLIP {
	 char path[MAXPATH];
	 enum process_type process_type;
	 enum stage enter;
	 enum stage exit;
	 enum content content;
	 enum angle angle;

	 CLIP() {
	  process_type = light;
	  enter = left;
	  exit = left;
	  content = child;
	  angle = section;
	  }
	};	
	
	BBXML();
	~BBXML();
	void setup();
	int processClips(string *path);
	void deleteClips();
	void processTimeRule();
	void processTransitionRule();

	std::vector <CLIP *> mClips;
};