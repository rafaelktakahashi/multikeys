// Made by Rafael Kenji Takahashi

#include "stdafx.h"

#include "Remapper.h"
#include "Keyboard.h"
#include "Level.h"
#include "Scancode.h"
#include "KeystrokeCommands.h"

// Xerces
#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>

// implementation of readSettings in Remapper class


// Typedefs
typedef xercesc::XercesDOMParser	XercesDOMParser, *PXercesDOMParser;
typedef xercesc::XMLString			XmlString, *PXmlString;
typedef xercesc::DOMDocument		XmlDocument, *PXmlDocument;
typedef xercesc::DOMNode			XmlNode, *PXmlNode;
typedef xercesc::DOMNodeList		XmlNodeList, *PXmlNodeList;
typedef xercesc::DOMAttr			XmlAttribute, *PXmlAttribute;
typedef xercesc::DOMNamedNodeMap	XmlNamedNodeMap, *PXmlNamedNodeMap;
typedef xercesc::DOMElement			XmlElement, *PXmlElement;


namespace Multikeys
{

	/*---Prototypes---*/

	// Parses an entire document node and extracts an array of keyboards from it.
	// PXmlDocument document - node representing the entire document to be parsed
	// keyboardArray - array of PKeyboard (Keyboard pointers) that will contain the final result
	// keyboardCount - will contain the amount of keyboards read (length of keyboard array)
	bool ParseDocument(PXmlDocument document,
		OUT PKeyboard **const keyboardArray,
		OUT unsigned int *const keyboardCount);

	// Parses a keyboard element and places its data in a Keyboard class;
	// pKeyboard - (pointer to) keyboard structure that will hold this node's data.
	bool ParseKeyboard(PXmlElement kbElement, OUT PKeyboard *const pKeyboard);

	// Receives a keyboard element, and instantiates a modifier state map with its remap in it
	bool ParseModifier(PXmlElement modElement, OUT ModifierStateMap* *const pModifiers);

	// Parses a level element and places its data in a Level class;
	// pLevel - (pointer to) level structure that will hold this node's data
	bool ParseLevel(PXmlElement lvlElement, OUT PLevel *const pLevel, ModifierStateMap *const pModifiers);
	bool ParseUnicode(PXmlElement rmpElement, OUT PKeystrokeCommand *const pCommand);
	bool ParseMacro(PXmlElement rmpElement, OUT PKeystrokeCommand *const pCommand);
	bool ParseExecutable(PXmlElement rmpElement, OUT PKeystrokeCommand *const pCommand);
	bool ParseDeadKey(PXmlElement rmpElement, OUT PKeystrokeCommand *const pCommand);


	bool Remapper::loadSettings(const std::wstring filename)
	{
		try
		{
			xercesc::XMLPlatformUtils::Initialize();
		}
		catch (const xercesc::XMLException& e)
		{
			OutputDebugString(L"Error during initialization of Xerces: " + e.getMessage + L"\n");
			return false;
		}

		PXercesDOMParser parser = new XercesDOMParser();
		parser->setValidationScheme(XercesDOMParser::Val_Always);
		parser->setDoNamespaces(true);
		parser->setCreateCommentNodes(false);		// Do not create comments

		xercesc::ErrorHandler* errorHandler = (xercesc::ErrorHandler*) new xercesc::HandlerBase();
		parser->setErrorHandler(errorHandler);

		// Load the xml file at filename and generate a tree
		try
		{
			parser->parse(filename.c_str());
		}
		catch (const xercesc::XMLException& e)
		{
			OutputDebugString(L"Error at Xerces, failed to parse file: " + e.getMessage + L"\n");
			return false;
		}


		// Do the actual processing
		PXmlDocument document = parser->getDocument();	// Root of the document to be parsed

		// Actually loading stuff into this parser's list of keyboards is delegated into another function:
		PKeyboard* keyboards = nullptr;
		unsigned int keyboardCount = 0;
		ParseDocument(document, &keyboards, &keyboardCount);
		if (keyboards == nullptr) return false;

		// Set!
		this->keyboards.clear();
		this->keyboards.assign(keyboards, keyboards + keyboardCount);

		// At the very end
		document->release();
		delete parser;
		delete errorHandler;
		try
		{
			xercesc::XMLPlatformUtils::Terminate();
		}
		catch (xercesc::XMLException& e)
		{
			OutputDebugString(L"Xerces teardown error: " + e.getMessage + L"\n");
			return false;
		}

		return true;
	}



	bool ParseDocument(PXmlDocument document,
		OUT PKeyboard **const keyboardArray,
		OUT unsigned int *const keyboardCount)
	{
		// Get root element
		PXmlElement root = document->getDocumentElement();

		// Get all children elements named "keyboard"
		PXmlNodeList keyboardElements = document->getElementsByTagName(L"keyboard");

		// keyboardArray is Keyboard***const :)
		*keyboardCount = keyboardElements->getLength();
		*keyboardArray = new PKeyboard[*keyboardCount];
		// At this point, *keyboardArray is array of PKeyboard,
		//		but each PKeyboard is itself an uninitialized pointer.

		for (XMLSize_t i = 0; i < keyboardElements->getLength(); i++)
		{
			// Someone told me "if your program needs to know what class an object is,
			//		that usually indicates a design flaw". We do this cast here because
			//		Xerces' getElementsByTagName returns Nodes that are all Elements.
			if (keyboardElements->item(i)->getNodeType() != xercesc::DOMNode::ELEMENT_NODE)
				return false;
			PXmlElement keyboardElement = (PXmlElement)keyboardElements->item(i);

			// 1. keyboardArray is a pointer to an array of PKeyboards (PKeyboard **const)
			// 2. *keyboardArray is the array of PKeyboards, already allocated
			// 3. (*keyboardArray)[i] is one PKeyboard in that array, at position i
			// 4. &((*keyboardArray)[i]) is a reference to a PKeyboard at position i
			// 5. After this call, (*keyboardArray)[i] will be a PKeyboard pointing to
			//			an instantiated Keyboard structure.
			if ( !ParseKeyboard(keyboardElement, &((*keyboardArray)[i])) )
				return false;
		}

		return true;
	}


	bool ParseKeyboard(PXmlElement kbElement, OUT PKeyboard *const pKeyboard)
	{

		// Get name
		const wchar_t* keyboardName = kbElement->getAttribute(L"Name");
		// Keyboards also have an alias attribute, but that's for the UI

		// There should be one "modifiers" tag:
		PXmlNodeList modifierElements = kbElement->getElementsByTagName(L"modifiers");
		if (modifierElements->getLength() != 1)
			return false;
		PXmlNode modifierElement = modifierElements->item(0);
		if (modifierElement->getNodeType != XmlNode::ELEMENT_NODE)
			return false;
		// Get pointer to modifier state map
		ModifierStateMap * ptrModStateMap;
		// Pass it to the function that will instantiate it
		if ( !ParseModifier((PXmlElement)modifierElement, &ptrModStateMap) )
			return false;
		// modStateMap should now contain an instantiated a modifier state map.
		// We'll instantiate it after making all levels


		// Get all levels
		PXmlNodeList levelElements = kbElement->getElementsByTagName(L"level");

		// Allocate memory for levels
		PLevel* levelArray = new PLevel[levelElements->getLength()];

		for (XMLSize_t i = 0; i < levelElements->getLength(); i++)
		{
			if (levelElements->item(i)->getNodeType() != XmlNode::ELEMENT_NODE)
				return false;	// there was a "level" that's not an element
			PXmlElement levelElement = (PXmlElement)levelElements->item(i);

			if ( !ParseLevel(levelElement, &(levelArray[i]), ptrModStateMap) )
				return false;
		}

		// levelArray is ready, and so is the modifier state map
		*pKeyboard =
			new Keyboard(levelElements->getLength(), levelArray, ptrModStateMap);

		return true;
	}



	bool ParseModifier(PXmlElement modElement, OUT ModifierStateMap* *const pModifiers)
	{
		// This element contains any number of "modifier" tags in it
		PXmlNodeList modifierElements = modElement->getElementsByTagName(L"modifier");

		// Get a multimap to place stuff in
		std::multimap<std::wstring, unsigned int> modMultimap;

		for (XMLSize_t i = 0; i < modifierElements->getLength(); i++)
		{
			// for each element, place a pair in the multimap containing its name and value
			if (modifierElements->item(i)->getNodeType != XmlNode::ELEMENT_NODE)
				return false;		// there was a "modifier" that's not an element
			PXmlElement thisElement = (PXmlElement)modifierElements->item(i);
			// get name
			std::wstring modifierName = std::wstring(thisElement->getAttribute(L"Name"));
			// get value
			std::wstring modifierValue = std::wstring(thisElement->getNodeValue());
			unsigned int iModifierValue = 0;
			try
			{
				iModifierValue = std::stoi(modifierValue, 0, 16);
			}
			catch (std::exception e)
			{
				// handle errors
				return false;
			}
			auto thisPair =
				std::pair<std::wstring, unsigned int>(modifierName, iModifierValue);
			modMultimap.insert(thisPair);

		}


		// modMultimap contains a number of pairs equal to the amount of modifiers read
		// some keys are the same; the amount of keys is equal to the amount of modifiers
		// pairs with the same key (name) are composite modifiers
		for (auto it = modMultimap.begin(); it != modMultimap.end(); it++)
		{
			;	// TODO
		}

		return true;
	}



	bool ParseLevel(PXmlElement lvlElement, OUT PLevel *const pLevel, ModifierStateMap *const pModifiers)
	{
		// Get a copy of the modifier state map
		ModifierStateMap * ptrLevelModMap = new ModifierStateMap(*pModifiers);

		// Get all modifiers that are required to trigger this level
		PXmlNodeList modifierList = lvlElement->getElementsByTagName(L"modifier");
		for (XMLSize_t i = 0; i < modifierList->getLength(); i++)
		{
			PXmlNode modifier = modifierList->item(i);
			if (modifier->getNodeType() != XmlNode::NodeType::ELEMENT_NODE)
				return false;		// there was a non-element "modifier"
			std::wstring modifierName = std::wstring(modifier->getNodeValue);
			ptrLevelModMap->setState(modifierName, true);
		}

		// Read all remaps
		PXmlNodeList allChildren = lvlElement->getChildNodes();
		std::vector<Scancode> allScancodes;
		std::vector<PKeystrokeCommand> allCommands;

		for (XMLSize_t i = 0; i < modifierList->getLength; i++)
		{
			PXmlNode child = allChildren->item(i);
			if (child->getNodeType() != XmlNode::NodeType::ELEMENT_NODE)
				continue;

			std::wstring childTagName = std::wstring(child->getNodeName);
			PKeystrokeCommand commandPointer = nullptr;
			
			if (childTagName.compare(L"unicode") == 0)
			{
				if (!ParseUnicode((PXmlElement)child, &commandPointer))
					return false;
			}
			else if (childTagName.compare(L"macro") == 0)
			{
				if (!ParseMacro((PXmlElement)child, &commandPointer))
					return false;
			}
			else if (childTagName.compare(L"execute") == 0)
			{
				if (!ParseExecutable((PXmlElement)child, &commandPointer))
					return false;
			}
			else if (childTagName.compare(L"deadkey") == 0)
			{
				if (!ParseDeadKey((PXmlElement)child, &commandPointer))
					return false;
			}
			else return false;
			// commandPointer should contain a command now
			allCommands.push_back(commandPointer);

			std::wstring scancode = ((PXmlElement)child)->getAttribute(L"Scancode");
			try
			{
				unsigned short iScancode = std::stoi(scancode.c_str(), 0, 16);
				if (iScancode <= 0xFF)
				{
					Scancode sc = Scancode(iScancode & 0xFF);
					allScancodes.push_back(sc);
				}
				else
				{
					Scancode sc = Scancode(iScancode >> 8, iScancode & 0xFF);
					allScancodes.push_back(sc);
				}
			}
			catch (std::exception e)
			{
				// handle errors
				return false;
			}
			
			*pLevel =
				new Level(ptrLevelModMap, allCommands.size(), allScancodes.data(), allCommands.data());
			// It's okay that these arrays die at the end of this function.

		}


		return true;
	}




	bool ParseUnicode(PXmlElement rmpElement, OUT PKeystrokeCommand *const pCommand)
	{
		// retrieve trigger on repeat attribute
		bool triggerOnRepeat =
			wcscmp(rmpElement->getAttribute(L"TriggerOnRepeat"), L"True") == 0;

		std::vector<unsigned int> codepointVector;

		// read each codepoint
		PXmlNodeList codepointElements = rmpElement->getElementsByTagName(L"codepoint");
		for (XMLSize_t i = 0; i < codepointElements->getLength(); i++)
		{
			if (codepointElements->item(i)->getNodeType != XmlNode::NodeType::ELEMENT_NODE)
				continue;		// found codepoint node that wasn't element

			try
			{
				unsigned int codepoint = std::stoi(
					codepointElements->item(i)->getNodeValue(), 0, 16
				);
				codepointVector.push_back(codepoint);
			}
			catch (std::exception e)
			{
				// handle errors
				return false;
			}
		}

		*pCommand =
			new UnicodeCommand(codepointVector.data(), codepointVector.size(), triggerOnRepeat);
		return true;

	}

	bool ParseMacro(PXmlElement rmpElement, OUT PKeystrokeCommand *const pCommand)
	{
		// retrieve trigger on repeat attribute
		bool triggerOnRepeat =
			wcscmp(rmpElement->getAttribute(L"TriggerOnRepeat"), L"True") == 0;

		std::vector<unsigned short> vkeyVector;

		// read each vKey
		PXmlNodeList vkeyElements = rmpElement->getElementsByTagName(L"vkey");
		for (XMLSize_t i = 0; i < vkeyElements->getLength(); i++)
		{
			if (vkeyElements->item(i)->getNodeType != XmlNode::NodeType::ELEMENT_NODE)
				continue;

			try
			{
				unsigned short vkey = std::stoi(
					vkeyElements->item(i)->getNodeValue(), 0, 16
				);

				bool isKeyUp =
					wcscmp(
						((PXmlElement)vkeyElements->item(i))->getAttribute(L"Keypress"),
						L"Up"
					) == 0;

				if (isKeyUp)		// turn on the most significant bit of a 16-bit variable
					vkey |= 0x8000;

				vkeyVector.push_back(vkey);
			}
			catch (std::exception e)
			{
				// handle errors
				return false;
			}
		}

		*pCommand =
			new MacroCommand(vkeyVector.data(), vkeyVector.size(), triggerOnRepeat);
		return true;
	}

	bool ParseExecutable(PXmlElement rmpElement, OUT PKeystrokeCommand *const pCommand)
	{
		// One "path" element, one "parameter" element
		PXmlNodeList pathElementList = rmpElement->getElementsByTagName(L"path");
		if (pathElementList->getLength() != 1)
			return false;
		if (pathElementList->item(0)->getNodeType() != XmlNode::NodeType::ELEMENT_NODE)
			return false;
		std::wstring path = pathElementList->item(0)->getNodeValue();

		PXmlNodeList parameterElementList = rmpElement->getElementsByTagName(L"parameter");
		if (parameterElementList->getLength() == 1)
		{
			if (parameterElementList->item(0)->getNodeType != XmlNode::NodeType::ELEMENT_NODE)
				return false;
			std::wstring parameter = parameterElementList->item(0)->getNodeValue();

			*pCommand =
				new ExecutableCommand(path, parameter);
		}
		else
		{
			*pCommand =
				new ExecutableCommand(path);
		}

		return true;
	}

	bool ParseIndependentCodepoints(
		PXmlElement indElement,
		OUT std::vector<unsigned short>* codepoints);
	bool ParseReplacements(
		PXmlElement replElement,
		OUT std::map<UnicodeCommand*, UnicodeCommand*>* replacements);
	bool ParseDeadKey(PXmlElement rmpElement, OUT PKeystrokeCommand *const pCommand)
	{
		std::vector<unsigned short> codepointVector;
		// These two make a lot more sense if they're maps, but there's no good way of
		// passing a c-style map (?) to a function
		std::vector<UnicodeCommand*> replacementsFromVector;
		std::vector<UnicodeCommand*> replacementsToVector;
		
		PXmlNodeList workList;

		workList = rmpElement->getElementsByTagName(L"independent");
		if (workList->getLength() != 1)
			return false;
		if (workList->item(1)->getNodeType() != XmlNode::NodeType::ELEMENT_NODE)
			return false;
		if (!ParseIndependentCodepoints((PXmlElement)workList->item(1), &codepointVector))
			return false;

		;


		
		// set dead key pointer
		return true;
	}



}		// End of namespace MutiKeys