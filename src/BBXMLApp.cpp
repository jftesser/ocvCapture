#include "BBXMLApp.hpp"

BBXML::BBXML() {
	setup();
}
BBXML::~BBXML() {
	deleteClips();
}

void BBXML::setup()
{
 int numClips;
 CLIP *clips[MAXCLIPS];
 string path="c:/bblibrary.xml";
 numClips=processClips(&path);
}

int BBXML::processClips(string *path)
{
	vector <CLIP *> clips;
	int index=0;
	//for debugging, load the hard coded file
	XmlTree doc ( loadFile(*path));	

	//get the library

	XmlTree lib = doc.getChild("Library");

	//iterate through the clips stored in the library
	for ( XmlTree::Iter iter = lib.begin(); iter != lib.end(); ++iter) {
		CLIP *newClip=new CLIP;
		
		//iterate through the child nodes of the clip
		XmlTree xmlClip = *iter; //XmlTree::Iter objects have no begin() or end() methods
		for( XmlTree::Iter child = xmlClip.begin(); child != xmlClip.end(); ++child ) {
			if (!child->getTag().compare("Path")) { strcpy_s((*newClip).path,MAXPATH,child->getValue().c_str());}
				
			if (!child->getTag().compare("ProcessType")) { 
				string val=child->getValue();
				if (val=="light") {(*newClip).process_type=light;}
				if (val=="dark") {(*newClip).process_type=dark;}
				if (val=="gray") {(*newClip).process_type=gray;}
				if (val=="spot") {(*newClip).process_type=spot;}
			}

			if (!child->getTag().compare("Enter")) { 
				string val=child->getValue();
				if (val=="left") {(*newClip).enter=left;}
				if (val=="center") {(*newClip).enter=center;}
				if (val=="right") {(*newClip).enter=right;}
				if (val=="any") {(*newClip).enter=any;}
			}

			if (!child->getTag().compare("Exit")) { 
				string val=child->getValue();
				if (val=="left") {(*newClip).exit=left;}
				if (val=="center") {(*newClip).exit=center;}
				if (val=="right") {(*newClip).exit=right;}
				if (val=="any") {(*newClip).exit=any;}
			}

			if (!child->getTag().compare("Content")) { 
				string val=child->getValue();
				if (val=="child") {(*newClip).content=content::child;}
				if (val=="teen") {(*newClip).content=teen;}
				if (val=="adult") {(*newClip).content=adult;}
			}	
		
			if (!child->getTag().compare("Angle")) { 
				string val=child->getValue();
				if (val=="section") {(*newClip).angle=section;}
				if (val=="axial") {(*newClip).angle=axial;}
			}	
		}
		clips.push_back(newClip);
		//when the loop finishes, index == the number of elements in clips[]
	}
	mClips = clips;
	return index;
}

void BBXML::deleteClips(){
	//essentially a destructor for the clip structs the clips[] points to

	for (int index=0;index<mClips.size();index++){
		delete mClips[index];
		mClips[index]=0;
	}
}
void BBXML::processTimeRule()
{
	struct TIME_RULE {
		double st;
		double en;
		enum process_type process_type;
		enum stage enter;
		enum stage exit;
		enum content content;
		enum angle angle;
	

		TIME_RULE() {
			st = -1.0;
			en = -1.0;
			process_type = light;
			enter = left;
			exit = left;
			content = child;
			angle = section;
		}
	};
	TIME_RULE newTIME_RULE;
	XmlTree doc (loadFile ("c:\test.xml"));
	XmlTree xmlTIME_RULE = doc.getChild("TIME_RULE");

	//consider converting to windows time
	newTIME_RULE.st = xmlTIME_RULE.getAttributeValue<double>("st");
	newTIME_RULE.en = xmlTIME_RULE.getAttributeValue<double>("en");
	/*
	newTIME_RULE.process_type = xmlTIME_RULE.getAttributeValue("process_type", -1);
	newTIME_RULE.enter = xmlTIME_RULE.getAttributeValue( "enter", -1);
	newTIME_RULE.exit = xmlTIME_RULE.getAttributeValue( "exit", -1);
	newTIME_RULE.content = xmlTIME_RULE.getAttributeValue( "content", -1);
	newTIME_RULE.angle = xmlTIME_RULE.getAttributeValue( "angle", -1);
	*/

}

void BBXML::processTransitionRule()
{
	struct TRANSITION_RULE {
		int from_process_type;
		int from_enter;
		int from_exit;
		int from_content;
		int from_angle;

		int to_process_type;
		int to_enter;
		int to_exit;
		int to_content;
		int to_angle;

	TRANSITION_RULE() {
		from_process_type = -1;
		from_enter = -1;
		from_exit = -1;
		from_content = -1;
		from_angle = -1;

		to_process_type = -1;
		to_enter = -1;
		to_exit = -1;
		to_content = -1;
		to_angle = -1;
		}
	};

	TRANSITION_RULE newTRANSITION_RULE;
	XmlTree doc (loadFile ("c:\test.xml"));
	XmlTree xmlTRANSITION_RULE = doc.getChild("TRANSITION_RULE");

	newTRANSITION_RULE.to_process_type = xmlTRANSITION_RULE.getAttributeValue("to_process_type", -1);
	newTRANSITION_RULE.to_enter = xmlTRANSITION_RULE.getAttributeValue( "to_enter", -1);
	newTRANSITION_RULE.to_exit = xmlTRANSITION_RULE.getAttributeValue( "to_exit", -1);
	newTRANSITION_RULE.to_content = xmlTRANSITION_RULE.getAttributeValue( "to_content", -1);
	newTRANSITION_RULE.to_angle = xmlTRANSITION_RULE.getAttributeValue( "to_angle", -1);

	newTRANSITION_RULE.from_process_type = xmlTRANSITION_RULE.getAttributeValue("from_process_type", -1);
	newTRANSITION_RULE.from_enter = xmlTRANSITION_RULE.getAttributeValue( "from_enter", -1);
	newTRANSITION_RULE.from_exit = xmlTRANSITION_RULE.getAttributeValue( "from_exit", -1);
	newTRANSITION_RULE.from_content = xmlTRANSITION_RULE.getAttributeValue( "from_content", -1);
	newTRANSITION_RULE.from_angle = xmlTRANSITION_RULE.getAttributeValue( "from_angle", -1);

}