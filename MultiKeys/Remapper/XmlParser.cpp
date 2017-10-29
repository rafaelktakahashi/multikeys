// Author: Rafael Kenji Takahashi, FATEC Mogi das Cruzes

#include "stdafx.h"

#include "Remapper.h"
#include "Keyboard.h"
#include "Layer.h"
#include "Scancode.h"
#include "KeystrokeCommands.h"

#include <stdexcept>
#include <algorithm>	// for string replacement

// Xerces
#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>


// helper function to convert a const XMLCh* into a wchar_t*
const wchar_t* xmlch_to_wcs(const XMLCh* from)
{
	return reinterpret_cast<const wchar_t*>(from);
}

// helper function to build a wstring from a const XMLCh*.
std::wstring xmlch_to_wstring(const XMLCh* from)
{
	if (!from) throw std::invalid_argument("XMLCh* argument was null.");
	auto u16 = std::u16string(from);
	return std::wstring(u16.begin(), u16.end());
}

// helper function to compare an XMLCh* with a wide string
int u16wcscmp(const XMLCh* first, const wchar_t* second)
{
	return wcscmp(
		xmlch_to_wcs(first),
		second
	);
}

// implementation of readSettings in Remapper class


// Typedefs for ease of use
typedef xercesc::XercesDOMParser	XercesDOMParser, *PXercesDOMParser;
typedef xercesc::XMLString			XmlString, *PXmlString;
typedef xercesc::DOMDocument		XmlDocument, *PXmlDocument;
typedef xercesc::DOMNode			XmlNode, *PXmlNode;
typedef xercesc::DOMNodeList		XmlNodeList, *PXmlNodeList;
typedef xercesc::DOMAttr			XmlAttribute, *PXmlAttribute;
typedef xercesc::DOMNamedNodeMap	XmlNamedNodeMap, *PXmlNamedNodeMap;
typedef xercesc::DOMElement			XmlElement, *PXmlElement;

using namespace Multikeys;


/*---Prototypes for functions used in this file---*/
// Parses an entire document node and extracts an array of keyboards from it.
// PXmlDocument document - node representing the entire document to be parsed
// keyboardArray - array of Keyboard* (Keyboard pointers) that will contain the final result
// keyboardCount - will contain the amount of keyboards read (length of keyboard array)
bool ParseDocument(const PXmlDocument document,
	OUT Keyboard* **const keyboardArray,
	OUT unsigned int *const keyboardCount);

// Parses a keyboard element and places its data in a Keyboard class;
// Keyboard* - (pointer to) keyboard structure that will hold this node's data.
bool ParseKeyboard(const PXmlElement kbElement, OUT Keyboard* *const pKeyboard);

// Receives a keyboard element, and instantiates a modifier state map with its remap in it
bool ParseModifier(const PXmlElement modElement, OUT ModifierStateMap* *const pModifiers);

// Parses a layer element and places its data in a Layer class;
// pLayer - (pointer to) layer structure that will hold this node's data
bool ParseLayer(const PXmlElement lvlElement, OUT Layer* *const pLayer);
bool ParseUnicode(const PXmlElement rmpElement, OUT BaseKeystrokeCommand* *const pCommand);
bool ParseMacro(const PXmlElement rmpElement, OUT BaseKeystrokeCommand* *const pCommand);
bool ParseExecutable(const PXmlElement rmpElement, OUT BaseKeystrokeCommand* *const pCommand);
bool ParseDeadKey(const PXmlElement rmpElement, OUT BaseKeystrokeCommand* *const pCommand);




namespace Multikeys
{
	bool Remapper::loadSettings(const std::wstring filename)
	{
		try
		{
			xercesc::XMLPlatformUtils::Initialize();
		}
		catch (const xercesc::XMLException&)
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
			parser->parse((XMLCh*)(filename.c_str()));	// this build of xerces expects char16_t
		}
		catch (const xercesc::XMLException&)
		{
			return false;
		}


		// Do the actual processing
		PXmlDocument document = parser->getDocument();	// Root of the document to be parsed

														// Actually loading stuff into this parser's list of keyboards is delegated into another function:
		Keyboard** keyboards = nullptr;
		unsigned int keyboardCount = 0;
		ParseDocument(document, &keyboards, &keyboardCount);
		if (keyboards == nullptr)
		{
			OutputDebugString(L"No keyboard found!");
			return false;
		}

		// Set!
		this->keyboards.clear();		// 'this' refers to this instance of Remapper.
		this->keyboards.assign(keyboards, keyboards + keyboardCount);

																		// At the very end
		
		// apparently we can't free the parser and also release the document. Doing both causes an exception.
		// document->release();
		delete parser;
		delete errorHandler;
		
		try
		{
			xercesc::XMLPlatformUtils::Terminate();
		}
		catch (xercesc::XMLException&)
		{
			return false;
		}

		return true;
	}
}





/* Implementing prototypes */


bool ParseDocument(const PXmlDocument document,
	OUT Keyboard* **const keyboardArray,
	OUT unsigned int *const keyboardCount)
{
	// Get root element
	PXmlElement root = document->getDocumentElement();

	// Get all children elements named "keyboard"
	PXmlNodeList keyboardElements = document->getElementsByTagName(u"keyboard");

	// keyboardArray is Keyboard***const
	// first pointer is because it's an out parameter; where it points to is changed
	// second pointer is an array
	// third pointer is because the array contains pointers to Keyboard objects
	*keyboardCount = keyboardElements->getLength();
	*keyboardArray = new Keyboard*[*keyboardCount];
	// At this point, *keyboardArray is array of Keyboard*,
	//		but each Keyboard* is itself an uninitialized pointer.

	for (XMLSize_t i = 0; i < keyboardElements->getLength(); i++)
	{
		// We do this cast here because
		// Xerces' getElementsByTagName returns Nodes that are all Elements.
		if (keyboardElements->item(i)->getNodeType() != xercesc::DOMNode::ELEMENT_NODE)
			return false;
		PXmlElement keyboardElement = (PXmlElement)keyboardElements->item(i);

		// 1. keyboardArray is a pointer to an array of Keyboard*s (Keyboard* **const)
		// 2. *keyboardArray is the array of Keyboard*s, already allocated
		// 3. (*keyboardArray)[i] is one Keyboard* in that array, at position i
		// 4. &((*keyboardArray)[i]) is a reference to a Keyboard* at position i
		// 5. After this call, (*keyboardArray)[i] will be a Keyboard* pointing to
		//			an instantiated Keyboard structure.
		if (!ParseKeyboard(keyboardElement, &((*keyboardArray)[i])))
			return false;
	}

	return true;
}


bool ParseKeyboard(const PXmlElement kbElement, OUT Keyboard* *const pKeyboard)
{
	// Get name
	std::wstring keyboardName = xmlch_to_wstring( kbElement->getAttribute(u"Name") );
	// Keyboards also have an alias attribute, but that's for the UI

	// There should be one "modifiers" tag:
	PXmlNodeList modifierElements = kbElement->getElementsByTagName(u"modifiers");
	if (modifierElements->getLength() != 1)
		return false;
	PXmlNode modifierElement = modifierElements->item(0);
	if (modifierElement->getNodeType() != XmlNode::ELEMENT_NODE)
		return false;
	// Get pointer to modifier state map
	ModifierStateMap * ptrModStateMap = nullptr;
	// Pass it to the function that will instantiate it
	if (!ParseModifier((PXmlElement)modifierElement, &ptrModStateMap))
		return false;
	if (!ptrModStateMap) return false;
	// ptrModStateMap should now point to an instantiated modifier state map.
	// We'll instantiate it after making all layers


	// Get all layers
	PXmlNodeList layerElements = kbElement->getElementsByTagName(u"layer");

	// Allocate memory for layers
	std::vector<Layer*> layerVector;

	for (XMLSize_t i = 0; i < layerElements->getLength(); i++)
	{
		if (layerElements->item(i)->getNodeType() != XmlNode::ELEMENT_NODE)
			return false;	// there was a "layer" that's not an element
		PXmlElement layerElement = (PXmlElement)layerElements->item(i);

		// Declare a pointer to layer
		Layer* pLayer = nullptr;
		// This call will place an actual instance there
		if (!ParseLayer(layerElement, &pLayer))
			return false;
		// Add the new instance into the vector
		if (!pLayer) return false;
		layerVector.push_back(pLayer);
		// The pointer dies, but not the object.
	}

	// layerArray is ready, and so is the modifier state map
	*pKeyboard =
		new Keyboard(keyboardName, layerVector, ptrModStateMap);

	return true;
}



bool ParseModifier(const PXmlElement modElement, OUT ModifierStateMap**const pModifiers)
{
	// This element contains any number of "modifier" tags in it
	PXmlNodeList modifierElements = modElement->getElementsByTagName(u"modifier");

	// Get a multimap to place stuff in
	std::multimap<std::wstring, unsigned int> modMultimap;

	for (XMLSize_t i = 0; i < modifierElements->getLength(); i++)
	{
		// for each element, place a pair in the multimap containing its name and value
		if (modifierElements->item(i)->getNodeType() != XmlNode::ELEMENT_NODE)
			return false;		// there was a "modifier" that's not an element
		PXmlElement thisElement = (PXmlElement)modifierElements->item(i);
		// get name
		std::wstring modifierName = xmlch_to_wstring(thisElement->getAttribute(u"Name"));
		// get value
		std::wstring modifierScancode = xmlch_to_wstring(thisElement->getTextContent());
		unsigned int iModifierValue = 0;
		try
		{
			// Some modifiers have E0 flags; in that case, there will be a ':' complicating things,
			// so we must remove it to get a number like e038.
			modifierScancode.erase(std::remove(modifierScancode.begin(), modifierScancode.end(), L':'), modifierScancode.end());
			// Place the value (without the colon) into an integer
			iModifierValue = std::stoi(modifierScancode, 0, 16);
		}
		catch (std::exception e)
		{
			// handle errors
			return false;
		}
		auto thisPair =
			std::pair<std::wstring, unsigned int>(
				std::wstring(modifierName.begin(), modifierName.end()),
				iModifierValue
				);
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

	*pModifiers =
		new ModifierStateMap(modVector);

	return true;
}



bool ParseLayer(const PXmlElement lvlElement, OUT Layer** const pLayer)
{
	// Get a copy of the modifier state map
	// ModifierStateMap * ptrLayerModMap = new ModifierStateMap(*pModifiers);
	// no longer necessary to create a modifier state map

	// This will contain the modifiers that are necessary to trigger this layer,
	// identified only by name.
	std::vector<std::wstring> modifierCombination;

	// Get all modifiers that are required to trigger this layer
	PXmlNodeList modifierList = lvlElement->getElementsByTagName(u"modifier");
	for (XMLSize_t i = 0; i < modifierList->getLength(); i++)
	{
		PXmlNode modifier = modifierList->item(i);
		if (modifier->getNodeType() != XmlNode::NodeType::ELEMENT_NODE)
			return false;		// there was a non-element "modifier"
		std::wstring modifierName = xmlch_to_wstring(modifier->getTextContent());
		modifierCombination.push_back(modifierName);		// <- adds new modifier name to necessary modifiers
	}

	// At this point, modifierCombination contains the modifiers and will only go out of scope at
	// the end of this function.

	// Read all remaps
	PXmlNodeList allChildren = lvlElement->getChildNodes();
	std::unordered_map<Scancode, BaseKeystrokeCommand*> layout;

	for (XMLSize_t i = 0; i < allChildren->getLength(); i++)
	{
		PXmlNode child = allChildren->item(i);
		if (child->getNodeType() != XmlNode::NodeType::ELEMENT_NODE)
			continue;

		std::wstring childTagName = xmlch_to_wstring(child->getNodeName());
		BaseKeystrokeCommand* commandPointer = nullptr;

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
		// The only other kind of node that can appear is a modifier,
		// and those are read elsewhere.
		else continue;


		// commandPointer should contain a command now
		// no matter what kind of node was read (unicode, macro, etc), it must contain a Scancode attribute
		std::wstring scancode = xmlch_to_wstring(((PXmlElement)child)->getAttribute(u"Scancode"));
		// the bytes of a scancode may be optionally separated by a colon, in which case we remove it
		scancode.erase(std::remove(scancode.begin(), scancode.end(), L':'), scancode.end());
		try
		{
			unsigned short iScancode = std::stoi(scancode.c_str(), 0, 16);
			if (iScancode <= 0xFF)
			{													// remember that layout
				Scancode sc = Scancode(iScancode & 0xFF);		// is a map!
				layout[sc] = commandPointer;					// operator[] creates a
			}													// new entry.
			else
			{
				Scancode sc = Scancode(iScancode >> 8, iScancode & 0xFF);
				layout[sc] = commandPointer;
			}
		}
		catch (std::exception e)
		{
			// handle errors
			return false;
		}

	}

	*pLayer =
		new Layer(modifierCombination, layout);
	// It's okay that these containers die at the end of this function.
	// Layer will copy them in its constructor.

	return true;
}




bool ParseUnicode(const PXmlElement rmpElement, OUT BaseKeystrokeCommand* *const pCommand)
{
	// retrieve trigger on repeat attribute
	bool triggerOnRepeat =
		u16wcscmp(rmpElement->getAttribute(u"TriggerOnRepeat"), L"True") == 0;

	std::vector<unsigned int> codepointVector;

	// read each codepoint
	PXmlNodeList codepointElements = rmpElement->getElementsByTagName(u"codepoint");
	for (XMLSize_t i = 0; i < codepointElements->getLength(); i++)
	{
		if (codepointElements->item(i)->getNodeType() != XmlNode::NodeType::ELEMENT_NODE)
			continue;		// found codepoint node that wasn't element

		try
		{
			unsigned int codepoint = std::stoi(
				xmlch_to_wcs(codepointElements->item(i)->getTextContent()),
				0,
				16
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

bool ParseMacro(const PXmlElement rmpElement, OUT BaseKeystrokeCommand* *const pCommand)
{
	// retrieve trigger on repeat attribute
	bool triggerOnRepeat =
		u16wcscmp(rmpElement->getAttribute(u"TriggerOnRepeat"), L"True") == 0;

	std::vector<unsigned short> vkeyVector;

	// read each vKey
	PXmlNodeList vkeyElements = rmpElement->getElementsByTagName(u"vkey");
	for (XMLSize_t i = 0; i < vkeyElements->getLength(); i++)
	{
		if (vkeyElements->item(i)->getNodeType() != XmlNode::NodeType::ELEMENT_NODE)
			continue;

		try
		{
			unsigned short vkey = std::stoi(
				xmlch_to_wcs(vkeyElements->item(i)->getTextContent()),
				0,
				16
			);

			bool isKeyUp =
				u16wcscmp(
				((PXmlElement)vkeyElements->item(i))->getAttribute(u"Keypress"),
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

bool ParseExecutable(const PXmlElement rmpElement, OUT BaseKeystrokeCommand* *const pCommand)
{
	// One "path" element, one "parameter" element
	PXmlNodeList pathElementList = rmpElement->getElementsByTagName(u"path");
	if (pathElementList->getLength() != 1)
		return false;
	if (pathElementList->item(0)->getNodeType() != XmlNode::NodeType::ELEMENT_NODE)
		return false;
	std::wstring path = xmlch_to_wstring(pathElementList->item(0)->getTextContent());

	PXmlNodeList parameterElementList = rmpElement->getElementsByTagName(u"parameter");
	if (parameterElementList->getLength() == 1)
	{
		if (parameterElementList->item(0)->getNodeType() != XmlNode::NodeType::ELEMENT_NODE)
			return false;
		std::wstring parameter = xmlch_to_wstring(parameterElementList->item(0)->getTextContent());

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
	PXmlNodeList codepointElements = indElement->getElementsByTagName(u"codepoint");
	for (XMLSize_t i = 0; i < codepointElements->getLength(); i++)
	{
		if (codepointElements->item(i)->getNodeType() != XmlNode::NodeType::ELEMENT_NODE)
			continue;		// found codepoint node that wasn't element

		try
		{
			unsigned int codepoint = std::stoi(
				xmlch_to_wcs(codepointElements->item(i)->getTextContent()),
				0,
				16
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
	// Each node in replList is an element called <replacement> containing one <from> tag and a <to> tag.
	// Both the <from> and the <to> tag contain a list of <codepoint>s each.

	replacements->clear();
	for (XMLSize_t i = 0; i < replList->getLength(); i++)
	{
		if (replList->item(i)->getNodeType() != XmlNode::NodeType::ELEMENT_NODE)
			continue;

		// Retrieve both <from> and <to> tags
		// Put them in their own UnicodeCommands
		// Note: currently, dead keys can only map from unicode commands to other unicode commands.
		PXmlElement replacementNode = (PXmlElement)replList->item(i);
		// From
		std::vector<unsigned int> pFromCodepoints;
		workList = replacementNode->getElementsByTagName(u"from");
		if (workList->getLength() != 1) return false;
		if (workList->item(0)->getNodeType() != XmlNode::NodeType::ELEMENT_NODE) return false;
		if (!ParseIndependentCodepoints((PXmlElement)workList->item(0), &pFromCodepoints)) return false;
		// To
		std::vector<unsigned int> pToCodepoints;
		workList = replacementNode->getElementsByTagName(u"to");
		if (workList->getLength() != 1) return false;
		if (workList->item(0)->getNodeType() != XmlNode::NodeType::ELEMENT_NODE) return false;
		if (!ParseIndependentCodepoints((PXmlElement)workList->item(0), &pToCodepoints)) return false;
		// make both Unicode commands, store them in the map
		UnicodeCommand * pFromCommand = new UnicodeCommand(pFromCodepoints, true);
		UnicodeCommand * pToCommand = new UnicodeCommand(pToCodepoints, true);
		(*replacements)[pFromCommand] = pToCommand;		// These UnicodeCommands don't die
	}
	// replacements have already been inserted
	return true;
}
bool ParseDeadKey(const PXmlElement rmpElement, OUT BaseKeystrokeCommand* *const pCommand)
{
	// Unicode characters that represent this key independently
	std::vector<unsigned int> codepointVector;

	// Map that contains all replacements this dead key can make
	std::unordered_map<UnicodeCommand*, UnicodeCommand*> replacementsMap;

	// Retrieve independent codepoints
	PXmlNodeList workList;

	workList = rmpElement->getElementsByTagName(u"independent");
	if (workList->getLength() != 1)
		return false;
	if (workList->item(0)->getNodeType() != XmlNode::NodeType::ELEMENT_NODE)
		return false;
	if (!ParseIndependentCodepoints((PXmlElement)workList->item(0), &codepointVector))
		return false;
	// At this point, codepointVector contains a valid Unicode sequence

	// Retrieve replacements
	workList = rmpElement->getElementsByTagName(u"replacement");
	/*if (workList->getLength() != 1)	// nonsense; there may be many replacements
		return false;*/
	/*if (workList->item(0)->getNodeType() != XmlNode::NodeType::ELEMENT_NODE)
		return false;*/
	if (!ParseReplacements(workList, &replacementsMap))
		return false;
	// At this point, replacementsMap contains valid replacements

	// set dead key pointer
	*pCommand =
		new DeadKeyCommand(codepointVector, replacementsMap);
	return true;
}