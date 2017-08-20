// Author: Rafael Kenji Takahashi, FATEC Mogi das Cruzes

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
	bool ParseDocument(const PXmlDocument document,
		OUT PKeyboard **const keyboardArray,
		OUT unsigned int *const keyboardCount);

	// Parses a keyboard element and places its data in a Keyboard class;
	// pKeyboard - (pointer to) keyboard structure that will hold this node's data.
	bool ParseKeyboard(const PXmlElement kbElement, OUT PKeyboard *const pKeyboard);

	// Receives a keyboard element, and instantiates a modifier state map with its remap in it
	bool ParseModifier(const PXmlElement modElement, OUT ModifierStateMap* *const pModifiers);

	// Parses a level element and places its data in a Level class;
	// pLevel - (pointer to) level structure that will hold this node's data
	bool ParseLevel(const PXmlElement lvlElement, OUT Level* *const pLevel, ModifierStateMap *const pModifiers);
	bool ParseUnicode(const PXmlElement rmpElement, OUT PKeystrokeCommand *const pCommand);
	bool ParseMacro(const PXmlElement rmpElement, OUT PKeystrokeCommand *const pCommand);
	bool ParseExecutable(const PXmlElement rmpElement, OUT PKeystrokeCommand *const pCommand);
	bool ParseDeadKey(const PXmlElement rmpElement, OUT PKeystrokeCommand *const pCommand);


	bool Remapper::loadSettings(const std::wstring filename)
	{
		try
		{
			xercesc::XMLPlatformUtils::Initialize();
		}
		catch (const xercesc::XMLException& e)
		{
			// OutputDebugString(L"Error during initialization of Xerces: " + e.getMessage() + L"\n");
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
			parser->parse(filename.c_str());	// xerces expects a wchar_t*, not wstring
		}
		catch (const xercesc::XMLException& e)
		{
			// OutputDebugString(L"Error at Xerces, failed to parse file: " + e.getMessage() + L"\n");
			return false;
		}


		// Do the actual processing
		PXmlDocument document = parser->getDocument();	// Root of the document to be parsed

		// Actually loading stuff into this parser's list of keyboards is delegated into another function:
		PKeyboard* keyboards = nullptr;
		unsigned int keyboardCount = 0;
		ParseDocument(document, &keyboards, &keyboardCount);
		if (keyboards == nullptr)
		{
			OutputDebugString(L"No keyboard found!");
			return false;
		}

		// Set!
		this->keyboards.clear();		// Remember that this method is an implementation of Remapper::loadSettings()
		this->keyboards.assign(keyboards, keyboards + keyboardCount);	// and thus "this" is the remapper

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
			// OutputDebugString(L"Xerces teardown error: " + e.getMessage() + L"\n");
			return false;
		}

		return true;
	}



	bool ParseDocument(const PXmlDocument document,
		OUT PKeyboard **const keyboardArray,
		OUT unsigned int *const keyboardCount)
	{
		// Get root element
		PXmlElement root = document->getDocumentElement();

		// Get all children elements named "keyboard"
		PXmlNodeList keyboardElements = document->getElementsByTagName(L"keyboard");

		// keyboardArray is Keyboard***const
		// first pointer is because it's an out parameter; where it points to is changed
		// second pointer is an array
		// third pointer is because the array contains pointers to Keyboard objects
		*keyboardCount = keyboardElements->getLength();
		*keyboardArray = new PKeyboard[*keyboardCount];
		// At this point, *keyboardArray is array of PKeyboard,
		//		but each PKeyboard is itself an uninitialized pointer.

		for (XMLSize_t i = 0; i < keyboardElements->getLength(); i++)
		{
			// We do this cast here because
			// Xerces' getElementsByTagName returns Nodes that are all Elements.
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


	bool ParseKeyboard(const PXmlElement kbElement, OUT PKeyboard *const pKeyboard)
	{

		// Get name
		const wchar_t* keyboardName = kbElement->getAttribute(L"Name");
		// Keyboards also have an alias attribute, but that's for the UI

		// There should be one "modifiers" tag:
		PXmlNodeList modifierElements = kbElement->getElementsByTagName(L"modifiers");
		if (modifierElements->getLength() != 1)
			return false;
		PXmlNode modifierElement = modifierElements->item(0);
		if (modifierElement->getNodeType() != XmlNode::ELEMENT_NODE)
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
		Level** levelArray = new Level*[levelElements->getLength()];

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



	bool ParseModifier(const PXmlElement modElement, OUT ModifierStateMap**const pModifiers)
	{
		// This element contains any number of "modifier" tags in it
		PXmlNodeList modifierElements = modElement->getElementsByTagName(L"modifier");

		// Get a multimap to place stuff in
		std::multimap<std::wstring, unsigned int> modMultimap;

		for (XMLSize_t i = 0; i < modifierElements->getLength(); i++)
		{
			// for each element, place a pair in the multimap containing its name and value
			if (modifierElements->item(i)->getNodeType() != XmlNode::ELEMENT_NODE)
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

		std::vector<PModifier> modVector;		// getting a list of modifiers
		decltype(modMultimap.equal_range(L"")) range;	// range is of whatever type equal_range returns
			// range will become an std::pair of unknown.
		// range.second points to the first element that does not have a key of the specified parameter
		for (auto it = modMultimap.begin(); it != modMultimap.end(); it = range.second)
		{
			range = modMultimap.equal_range(it->first);
			// notice the step: range.second, which will go to the next key

			// Just figure out the length
			size_t length = 0;
			for (auto d = range.first; d != range.second; d++)
				length++;

			// see if a simple or composite modifier is needed
			PModifier pModifier;
			if (length == 1)		// simple modifier, there is one scancode
			{
				int scCode = range.first->second;
				if (scCode <= 0xff)	// One byte scancode
					pModifier = new SimpleModifier(it->first, Scancode(scCode));
				else				// Two byte scancode
					pModifier = new SimpleModifier(it->first, Scancode(scCode >> 8, scCode & 0xff));
			}
			else if (length > 1)	// composite modifier, there are many scancodes
			{
				std::vector<Scancode> scVector;	// temporarily hold scancodes
				for (auto d = range.first; d != range.second; d++)
				{
					if (d->second <= 0xff)	// One byte scancode
						scVector.push_back(Scancode(d->second));
					else					// Two byte scancode
						scVector.push_back(Scancode(d->second >> 8, d->second & 0xff));
				}
				pModifier = new CompositeModifier(it->first, scVector);
			}

			// add to the vector
			modVector.push_back(pModifier);
		}

		ModifierStateMap* modStateMap = new ModifierStateMap(modVector.size(), modVector.data());

		return true;
	}



	// Details - In addition to the node (from which to extract data) and a pointer to a level
	//		object (in which to write data), this method takes as argument a mutable modifier
	//		state map (only its pointer is constant). This level reuses that state map to represent
	//		which of those modifiers need to be on and which need to be off to trigger this level.
	bool ParseLevel(const PXmlElement lvlElement, OUT Level* *const pLevel, ModifierStateMap *const pModifiers)
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
			std::wstring modifierName = std::wstring(modifier->getNodeValue());
			ptrLevelModMap->setState(modifierName, true);
		}

		// Read all remaps
		PXmlNodeList allChildren = lvlElement->getChildNodes();
		std::vector<Scancode> allScancodes;
		std::vector<PKeystrokeCommand> allCommands;

		for (XMLSize_t i = 0; i < modifierList->getLength(); i++)
		{
			PXmlNode child = allChildren->item(i);
			if (child->getNodeType() != XmlNode::NodeType::ELEMENT_NODE)
				continue;

			std::wstring childTagName = std::wstring(child->getNodeName());
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




	bool ParseUnicode(const PXmlElement rmpElement, OUT PKeystrokeCommand *const pCommand)
	{
		// retrieve trigger on repeat attribute
		bool triggerOnRepeat =
			wcscmp(rmpElement->getAttribute(L"TriggerOnRepeat"), L"True") == 0;

		std::vector<unsigned int> codepointVector;

		// read each codepoint
		PXmlNodeList codepointElements = rmpElement->getElementsByTagName(L"codepoint");
		for (XMLSize_t i = 0; i < codepointElements->getLength(); i++)
		{
			if (codepointElements->item(i)->getNodeType() != XmlNode::NodeType::ELEMENT_NODE)
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

	bool ParseMacro(const PXmlElement rmpElement, OUT PKeystrokeCommand *const pCommand)
	{
		// retrieve trigger on repeat attribute
		bool triggerOnRepeat =
			wcscmp(rmpElement->getAttribute(L"TriggerOnRepeat"), L"True") == 0;

		std::vector<unsigned short> vkeyVector;

		// read each vKey
		PXmlNodeList vkeyElements = rmpElement->getElementsByTagName(L"vkey");
		for (XMLSize_t i = 0; i < vkeyElements->getLength(); i++)
		{
			if (vkeyElements->item(i)->getNodeType() != XmlNode::NodeType::ELEMENT_NODE)
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

	bool ParseExecutable(const PXmlElement rmpElement, OUT PKeystrokeCommand *const pCommand)
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
			if (parameterElementList->item(0)->getNodeType() != XmlNode::NodeType::ELEMENT_NODE)
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
		const PXmlElement indElement,
		OUT std::vector<unsigned int>* codepoints)
	{
		// Similar to Unicode command parsing
		std::vector<unsigned int> codepointVector;

		// read each codepoint
		PXmlNodeList codepointElements = indElement->getElementsByTagName(L"codepoint");
		for (XMLSize_t i = 0; i < codepointElements->getLength(); i++)
		{
			if (codepointElements->item(i)->getNodeType() != XmlNode::NodeType::ELEMENT_NODE)
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
		*codepoints = codepointVector;
		return true;
	}
	bool ParseReplacements(
		const PXmlNodeList replList,
		OUT std::unordered_map<UnicodeCommand*, UnicodeCommand*>* replacements)	// <-should not be null
	{
		PXmlNodeList workList;
		// Each node in replList is an element containing one <from> tag and a <to> tag.

		replacements->clear();
		for (XMLSize_t i = 0; i < replList->getLength(); i++)
		{
			if (replList->item(i)->getNodeType() != XmlNode::NodeType::ELEMENT_NODE)
				continue;

			// Retrieve both <from> and <to> tags
			// Put them in their own UnicodeCommands
			PXmlElement replacementNode = (PXmlElement)replList->item(i);
			// From
			std::vector<unsigned int>* pFromCodepoints;
			workList = replacementNode->getElementsByTagName(L"from");
			if (workList->getLength() != 1) return false;
			if (workList->item(1)->getNodeType() != XmlNode::NodeType::ELEMENT_NODE) return false;
			if (!ParseIndependentCodepoints((PXmlElement)workList->item(1), pFromCodepoints)) return false;
			// To
			std::vector<unsigned int>* pToCodepoints;
			workList = replacementNode->getElementsByTagName(L"to");
			if (workList->getLength() != 1) return false;
			if (workList->item(i)->getNodeType() != XmlNode::NodeType::ELEMENT_NODE) return false;
			if (!ParseIndependentCodepoints((PXmlElement)workList->item(1), pToCodepoints)) return false;
			// make both Unicode commands, store them in the map
			UnicodeCommand * pFromCommand = new UnicodeCommand(pFromCodepoints, true);
			UnicodeCommand * pToCommand = new UnicodeCommand(pToCodepoints, true);
			(*replacements)[pFromCommand] = pToCommand;		// These UnicodeCommands don't die
		}
		// replacements have already been inserted
		return true;
	}
	bool ParseDeadKey(const PXmlElement rmpElement, OUT PKeystrokeCommand *const pCommand)
	{
		// Unicode characters that represent this key independently
		std::vector<unsigned int> codepointVector;

		// Map that contains all replacements this dead key can make
		std::unordered_map<UnicodeCommand*, UnicodeCommand*> replacementsMap;

		// Retrieve independent codepoints
		PXmlNodeList workList;

		workList = rmpElement->getElementsByTagName(L"independent");
		if (workList->getLength() != 1)
			return false;
		if (workList->item(1)->getNodeType() != XmlNode::NodeType::ELEMENT_NODE)
			return false;
		if (!ParseIndependentCodepoints((PXmlElement)workList->item(1), &codepointVector))
			return false;
		// At this point, codepointVector contains a valid Unicode sequence

		// Retrieve replacements
		workList = rmpElement->getElementsByTagName(L"replacement");
		if (workList->getLength() != 1)
			return false;
		if (workList->item(1)->getNodeType() != XmlNode::NodeType::ELEMENT_NODE)
			return false;
		if (!ParseReplacements(workList, &replacementsMap))
			return false;
		// At this point, replacementsMap contains valid replacements

		// set dead key pointer
		*pCommand =
			new DeadKeyCommand(&codepointVector, &replacementsMap);
		return true;
	}





}		// End of namespace MutiKeys