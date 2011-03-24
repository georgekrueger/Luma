/*
  ==============================================================================

   Demonstration "Hello World" application in JUCE
   Copyright 2008 by Julian Storer.

  ==============================================================================
*/

#include "juce_AppConfig.h"
#include "juce_amalgamated.h"
#include "LumaPlug.h"

class PlugWindow  : public DocumentWindow
{
public:
    //==============================================================================
    PlugWindow(Component* const uiComp, String windowTitle) 
        : DocumentWindow (windowTitle,
                          Colours::lightgrey, 
                          DocumentWindow::allButtons,
                          true)
    {
        setContentComponent (uiComp, true, true);

        centreWithSize (getWidth(), getHeight());

        setVisible (true);
    }

    ~PlugWindow()
    {
		setContentComponent(0);
    }

    //==============================================================================
    void closeButtonPressed()
    {
        // When the user presses the close button, we'll tell the app to quit. This 
        // window will be deleted by our HelloWorldApplication::shutdown() method
        // 
        JUCEApplication::quit();
    }
};

class CommandIDs
{
public:
	static const int open                   = 0x30000;
	static const int save                   = 0x30001;
	static const int saveAs                 = 0x30002;
	static const int showPluginListEditor   = 0x30100;
	static const int showAudioSettings      = 0x30200;
	static const int aboutBox               = 0x30300;
	static const int play					= 0x30400;
	static const int stop					= 0x30500;
};


class LumaMenuBarModel : public MenuBarModel
{
public:
	LumaMenuBarModel(ApplicationCommandManager* aCommandManager) 
		: MenuBarModel(), commandManager(aCommandManager) 
	{
	}
	
	~LumaMenuBarModel() {}
	
private:
	ApplicationCommandManager* commandManager;
	
public:
	
	// --------------------------------------------------------
	// MenuBarModel Implementation
	// --------------------------------------------------------
	
	const StringArray getMenuBarNames()
	{
		const tchar* const names[] = { T("File"), T("Transport"), T("Options"), 0 };

		return StringArray ((const tchar**) names);
	}

	const PopupMenu getMenuForIndex (int topLevelMenuIndex, const String& menuName)
	{
		PopupMenu menu;

		if (topLevelMenuIndex == 0)
		{
			// "File" menu
			menu.addCommandItem (commandManager, CommandIDs::open);
			menu.addCommandItem (commandManager, CommandIDs::save);
			menu.addCommandItem (commandManager, CommandIDs::saveAs);
			menu.addSeparator();
			menu.addCommandItem (commandManager, StandardApplicationCommandIDs::quit);
		}
		else if (topLevelMenuIndex == 1)
		{
			// "Transport" menu
			menu.addCommandItem (commandManager, CommandIDs::play);
			menu.addCommandItem (commandManager, CommandIDs::stop);
		}
		else if (topLevelMenuIndex == 2)
		{
			// "Options" menu

			//menu.addCommandItem (commandManager, CommandIDs::showPluginListEditor);		
			menu.addCommandItem (commandManager, CommandIDs::showAudioSettings);

			//menu.addSeparator();
			//menu.addCommandItem (commandManager, CommandIDs::aboutBox);
		}

		return menu;
	}

	void menuItemSelected (int menuItemID, int topLevelMenuIndex)
	{
	}

};

class LumaPlayHead : public AudioPlayHead
{
public:
	LumaPlayHead()
		: AudioPlayHead()
	{
		posInfo.bpm = 120;
		posInfo.timeSigNumerator = 4;
		posInfo.timeSigDenominator = 4;
		posInfo.isPlaying = false;
		posInfo.isRecording = false;	
	}

	bool getCurrentPosition (CurrentPositionInfo& result)
	{
		printf("Called getCurrentPosition\n");
		result = posInfo;
		return true;
	}
	
	CurrentPositionInfo posInfo;
};

//==============================================================================
/** This is the application object that is started up when Juce starts. It handles
    the initialisation and shutdown of the whole application.
*/
class LumaApplication : public JUCEApplication
{
    /* Important! NEVER embed objects directly inside your JUCEApplication class! Use
       ONLY pointers to objects, which you should create during the initialise() method
       (NOT in the constructor!) and delete in the shutdown() method (NOT in the
       destructor!)

       This is because the application object gets created before Juce has been properly
       initialised, so any embedded objects would also get constructed too soon.
   */
	LumaPlug* lumaPlug;
	AudioPluginInstance* instrPlug;
	AudioProcessorEditor* lumaPlugEditor;
	AudioProcessorEditor* instrPlugEditor;
    PlugWindow* lumaWindow;
	PlugWindow* instrWindow;
	AudioDeviceManager* deviceManager;
	AudioProcessorGraph* graph;
	AudioProcessorGraph::AudioGraphIOProcessor* audioOut;
	AudioProcessorGraph::AudioGraphIOProcessor* midiIn;
	AudioProcessorPlayer* player;
	ApplicationCommandManager* commandManager;
	LumaMenuBarModel* menuBarModel;
	LumaPlayHead* playHead;


public:
    //==============================================================================
    LumaApplication()
        : lumaWindow (0), instrWindow(0)
    {
        // NEVER do anything in here that could involve any Juce function being called
        // - leave all your startup tasks until the initialise() method.
    }

    ~LumaApplication()
    {
        // Your shutdown() method should already have done all the things necessary to
        // clean up this app object, so you should never need to put anything in
        // the destructor.

        // Making any Juce calls in here could be very dangerous...
    }

    //==============================================================================
    void initialise (const String& commandLine)
    {
		// initialise our settings file..
        ApplicationProperties::getInstance()
            ->setStorageParameters (T("Luma Standalone"),
                                    T("settings"), String::empty, 1000,
                                    PropertiesFile::storeAsXML);
		
		
		// initialize audio device settings
		XmlElement* const savedAudioState = ApplicationProperties::getInstance()->getUserSettings()
                                            ->getXmlValue (T("audioDeviceState"));
		
		// create audio device manager
		deviceManager = new AudioDeviceManager;
		deviceManager->initialise (256, 256, savedAudioState, true);
		printf("Current Audio Device: %s\n", deviceManager->getCurrentAudioDeviceName().toUTF8());

		delete savedAudioState;
	
		// init command manager
		commandManager = new ApplicationCommandManager();
		commandManager->registerAllCommandsForTarget(this);
		menuBarModel = new LumaMenuBarModel(commandManager);
		
		// create the luma plugin and instrument plugin
		lumaPlug = new LumaPlug;
		instrPlug = getVstPlugInstance( File("/Users/george/Library/Audio/Plug-Ins/VST/mda_vst_ub/mda Piano.vst") );
		lumaPlugEditor = getPlugUI( lumaPlug );
		instrPlugEditor = getPlugUI( instrPlug );
		// create plugin windows.  plugin editor will be deleted when the window is closed
		lumaWindow = new PlugWindow( lumaPlugEditor, lumaPlug->getName() );
        instrWindow = new PlugWindow( instrPlugEditor, instrPlug->getName() );
		lumaWindow->addKeyListener (commandManager->getKeyMappings());
	
		playHead = new LumaPlayHead;
	
		// let the plugins know about the playhead
		lumaPlug->setPlayHead(playHead);
		instrPlug->setPlayHead(playHead);
		
		printf("luma plug: 0x%x\n", lumaPlug);
		printf("main playhead: 0x%x, luma playhead: 0x%x\n", playHead, lumaPlug->getPlayHead());

        
#if JUCE_MAC
		MenuBarModel::setMacMainMenu (menuBarModel);
#else
		lumaWindow->setMenuBar (menuBarModel);
#endif
		

		// create an audio processor graph and connect up luma to instrument
		graph = new AudioProcessorGraph;
		player = new AudioProcessorPlayer;
		
		player->setProcessor(graph);
		deviceManager->setAudioCallback(player);
		
		printf("Graph.  Inputs: %d, Outputs: %d\n", graph->getNumInputChannels(), graph->getNumOutputChannels());
		
		audioOut = new AudioProcessorGraph::AudioGraphIOProcessor(AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode);
		printf("Audio Out.  Inputs: %d, Outputs: %d\n", audioOut->getNumInputChannels(), audioOut->getNumOutputChannels());
		midiIn = new AudioProcessorGraph::AudioGraphIOProcessor(AudioProcessorGraph::AudioGraphIOProcessor::midiInputNode);
		
		
		// create graph nodes
		AudioProcessorGraph::Node* lumaNode = graph->addNode(lumaPlug);
		AudioProcessorGraph::Node* instrNode = graph->addNode(instrPlug);
		AudioProcessorGraph::Node* audioOutNode = graph->addNode(audioOut);
		if (!audioOutNode)
		{
			printf("UNABLE TO CREATE AUDIO OUT NODE\n");
		}
		
		// connect luma midi output to instument midi input
		bool connected = false;
		
		connected = graph->addConnection(lumaNode->id, AudioProcessorGraph::midiChannelIndex, instrNode->id, AudioProcessorGraph::midiChannelIndex);
		if (!connected)
		{
			printf("FAILED TO CONNECT LUMA MIDI CHANNEL TO INSTR MIDI CHANNEL\n");
		}
		
		// connect instrument audio out to graph audio out
		connected = graph->addConnection(instrNode->id, 0, audioOutNode->id, 0);
		if (!connected)
		{
			printf("FAILED TO CONNECT INSTR AUDIO CH 0 TO AUDIO OUT CH 0\n");
		}
		
		connected = graph->addConnection(instrNode->id, 1, audioOutNode->id, 1);
		if (!connected)
		{
			printf("FAILED TO CONNECT INSTR AUDIO CH 1 TO AUDIO OUT CH 1\n");
		}
		
		// we want priority!!
		Process::setPriority (Process::HighPriority);
		
        /*  ..and now return, which will fall into to the main event
            dispatch loop, and this will run until something calls
            JUCEAppliction::quit().
        */
    }

    void shutdown()
    {
        // clear up..
		
		ApplicationProperties::getInstance()->closeFiles();
		
		deviceManager->setAudioCallback(0);
		deleteAndZero(deviceManager);
		
#if JUCE_MAC
		MenuBarModel::setMacMainMenu (0);
#else
		lumaWindow->setMenuBar (0);
#endif

		
	    deleteAndZero(lumaWindow);
		deleteAndZero(instrWindow);
				
		player->setProcessor(0);
		deleteAndZero(player);
		graph->clear();
		deleteAndZero(graph);
		
		deleteAndZero(playHead);
				
		deleteAndZero(menuBarModel);		
		deleteAndZero(commandManager);
    }

    //==============================================================================
    const String getApplicationName()
    {
        return T("Luma Standalone");
    }

    const String getApplicationVersion()
    {
        return T("1.0");
    }

    bool moreThanOneInstanceAllowed()
    {
        return true;
    }

    void anotherInstanceStarted (const String& commandLine)
    {
    }
	
	// get plugin instance
	static AudioPluginInstance* getVstPlugInstance (File plugFile)
	{
		
		AudioPluginInstance* inst = NULL;
		VSTPluginFormat vstFormat;
		OwnedArray <PluginDescription> plugDescriptions;
		vstFormat.findAllTypesForFile (plugDescriptions, plugFile);
		if (plugDescriptions.size() == 1)
		{
			printf("Plug found: %s\n", plugFile.getFileName().toUTF8());
			inst = vstFormat.createInstanceFromDescription (*plugDescriptions[0]);
		}
		return inst;
	}

	static AudioProcessorEditor* getPlugUI (AudioProcessor* inst)
	{
		AudioProcessorEditor* ui = inst->createEditorIfNeeded();
		if (!ui)
		{
			ui = new GenericAudioProcessorEditor (inst);
		}
		return ui;
	}
	
	void showAudioSettings()
	{
		AudioDeviceSelectorComponent audioSettingsComp (*deviceManager,
														0, 256,
														0, 256,
														true, true);

		audioSettingsComp.setSize (500, 350);

		DialogWindow::showModalDialog (T("Audio Settings"),
									   &audioSettingsComp,
									   lumaWindow,
									   Colours::azure,
									   true);

		XmlElement* const audioState = deviceManager->createStateXml();

		ApplicationProperties::getInstance()->getUserSettings()
			->setValue (T("audioDeviceState"), audioState);

		delete audioState;

		ApplicationProperties::getInstance()->getUserSettings()->saveIfNeeded();
	}
	
	
	
	//==============================================================================
	// ApplicationCommandTarget interface
	//==============================================================================	
	ApplicationCommandTarget* getNextCommandTarget()
	{
		return findFirstTargetParentComponent();
	}

	void getAllCommands (Array <CommandID>& commands)
	{
		// this returns the set of all commands that this target can perform..
		const CommandID ids[] = { CommandIDs::open,
								  CommandIDs::save,
								  CommandIDs::saveAs,
								  CommandIDs::showPluginListEditor,
								  CommandIDs::showAudioSettings,
								  CommandIDs::aboutBox,
								  CommandIDs::play,
								  CommandIDs::stop
								};

		commands.addArray (ids, numElementsInArray (ids));
	}

	void getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result)
	{
		const String category ("General");

		switch (commandID)
		{
		case CommandIDs::open:
			result.setInfo (T("Open..."),
							T("Open a lua script file"),
							category, 0);
			result.defaultKeypresses.add (KeyPress (T('o'), ModifierKeys::commandModifier, 0));
			break;

		case CommandIDs::save:
			result.setInfo (T("Save"),
							T("Saves the current lua script file"),
							category, 0);
			result.defaultKeypresses.add (KeyPress (T('s'), ModifierKeys::commandModifier, 0));
			break;

		case CommandIDs::saveAs:
			result.setInfo (T("Save As..."),
							T("Saves a copy of the current lua script file"),
							category, 0);
			result.defaultKeypresses.add (KeyPress (T('s'), ModifierKeys::shiftModifier | ModifierKeys::commandModifier, 0));
			break;

		case CommandIDs::showPluginListEditor:
			result.setInfo ("Edit the list of available plug-Ins...", String::empty, category, 0);
			//result.addDefaultKeypress (T('p'), ModifierKeys::commandModifier);
			break;

		case CommandIDs::showAudioSettings:
			result.setInfo ("Change the audio device settings", String::empty, category, 0);
			result.addDefaultKeypress (T('a'), ModifierKeys::commandModifier);
			break;

		case CommandIDs::aboutBox:
			result.setInfo ("About...", String::empty, category, 0);
			break;
			
		case CommandIDs::play:
			result.setInfo ("Play", String::empty, category, 0);
			result.addDefaultKeypress (KeyPress::returnKey, ModifierKeys::commandModifier);
			break;
			
		case CommandIDs::stop:
			result.setInfo ("Stop", String::empty, category, 0);
			result.addDefaultKeypress (KeyPress::returnKey, ModifierKeys::shiftModifier | ModifierKeys::commandModifier);
			break;

		default:
			break;
		}
	}

	bool perform (const InvocationInfo& info)
	{
		//GraphDocumentComponent* const graphEditor = getGraphEditor();

		switch (info.commandID)
		{
		case CommandIDs::open:
			if (lumaPlug->getDocument()->saveIfNeededAndUserAgrees() == FileBasedDocument::savedOk)
				lumaPlug->getDocument()->loadFromUserSpecifiedFile (true);
			
			break;

		case CommandIDs::save:
			lumaPlug->getDocument()->save (true, true);
			break;

		case CommandIDs::saveAs:
			lumaPlug->getDocument()->saveAs (File::nonexistent, true, true, true);
			break;

		case CommandIDs::showPluginListEditor:
			//if (PluginListWindow::currentPluginListWindow == 0)
			//	PluginListWindow::currentPluginListWindow = new PluginListWindow (knownPluginList);

			//PluginListWindow::currentPluginListWindow->toFront (true);
			break;

		case CommandIDs::showAudioSettings:
			showAudioSettings();
			break;

		case CommandIDs::aboutBox:
			{
	           //AboutBoxComponent aboutComp;
//
//				DialogWindow::showModalDialog (T("About"),
//											   &aboutComp,
//											   this, Colours::white,
//											   true, false, false);
	        }

			break;
			
		case CommandIDs::play:
			printf("Play\n");
			playHead->posInfo.isPlaying = true;
			break;
			
		case CommandIDs::stop:
			printf("Stop\n");
			playHead->posInfo.isPlaying = false;
			break;

		default:
			return false;
		}

		return true;
	}

};



//==============================================================================
// This macro creates the application's main() function..
START_JUCE_APPLICATION (LumaApplication)
