//// Implementation of BOOL Multikeys::Remapper::ParseSettings
//// Uses MSXML to read an xml file with remapping configurations
//
//// If anyone knows how to parse xml files with more elegance,
////		do try and replace this entire thing with a better
////		implementation.
//
#include "stdafx.h"
//#include "Remaps.h"
//// #include <comutil.h>
//// #include <MsXml6.h>
//#import "msxml6.dll" raw_interfaces_only
//// Using #import rather than #include
//// Microsoft doesn't recommend it, apparently. I care little.
//#include <string.h>	// for splitting strings
//#include <string>	// for parsing
//
//// For splitting strings also:
//#include <vector>
//#include <sstream>
//#include <codecvt>
//
//
//// using namespace MSXML2;		// <- causes ambiguity.
//
//
//// Macro that calls a COM method and jumps to CleanUp if it fails
//#define CHK_HR(stmt)	do { hr=(stmt); if (FAILED(hr)) goto CleanUp; } while (0)
//
//// Macro to verify memory allocation; jumps to CleanUp if out of memory and places result in hr
//#define CHK_ALLOC(p)	do { if (!(p)) { hr = E_OUTOFMEMORY; goto CleanUp; } } while (0)
//
//// Macro that releases a COM object if it's not already NULL
//#define SAFE_RELEASE(p)	do { if ((p)) { (p)->Release(); (p) = NULL; } } while (0)
//
//
//// Typedefs for MSXML namespace
//typedef MSXML2::IXMLDOMDocument3		XmlDocument,		*XmlDocumentPtr;
//typedef MSXML2::IXMLDOMNode				XmlNode,			*XmlNodePtr;
//typedef MSXML2::IXMLDOMNodeList			XmlNodeList,		*XmlNodeListPtr;
//typedef MSXML2::IXMLDOMAttribute		XmlAttribute,		*XmlAttributePtr;
//typedef MSXML2::IXMLDOMNamedNodeMap		XmlNamedNodeMap,	*XmlNamedNodeMapPtr;
//typedef MSXML2::IXMLDOMElement			XmlElement,			*XmlElementPtr;
//typedef MSXML2::IXMLDOMParseError		XmlParseError,		*XmlParseErrorPtr;
//typedef MSXML2::DOMDocument60			Document60;
//typedef MSXML2::DOMNodeType				NodeType;
//
//
//// Helper function to create a VT_BSTR variant from a null terminated string (PCWSTR = WCHAR*)
//HRESULT VariantFromString(PCWSTR wszValue, VARIANT &Variant)
//{
//	HRESULT hr = S_OK;
//	BSTR bstr = SysAllocString(wszValue);
//	CHK_ALLOC(bstr);
//	V_VT(&Variant) = VT_BSTR;
//	V_BSTR(&Variant) = bstr;
//
//CleanUp:
//	return hr;
//}
//
//// Helper function to create a DOM instance for a document
//HRESULT CreateAndInitDOM(XmlDocumentPtr * ppDoc)
//{
//	HRESULT hr = CoCreateInstance(__uuidof(Document60), NULL,
//		CLSCTX_INPROC_SERVER, IID_PPV_ARGS(ppDoc));
//	if (SUCCEEDED(hr))
//	{
//		// these methods should not fail, so don't inspect result
//		(*ppDoc)->put_async(VARIANT_FALSE);
//		(*ppDoc)->put_validateOnParse(VARIANT_FALSE);
//		(*ppDoc)->put_resolveExternals(VARIANT_FALSE);
//	}
//	return hr;
//}
//
//// Function that loads a document object model
//// If a DOM is returned, call release on it when done
//XmlDocumentPtr loadDom(std::wstring file)
//{
//	HRESULT hr = S_OK;
//	XmlDocumentPtr pXMLDom = NULL;
//	XmlParseErrorPtr pXMLErr = NULL;
//
//	BOOL flgGoodToReturn = FALSE;
//
//	BSTR bstrXML = NULL;
//	BSTR bstrErr = NULL;
//	VARIANT_BOOL varStatus;
//	VARIANT varFileName;
//	VariantInit(&varFileName);
//
//	CHK_HR(CreateAndInitDOM(&pXMLDom));
//
//	// CML file name to load
//	CHK_HR(VariantFromString(file.c_str(), varFileName));
//	CHK_HR(pXMLDom->load(varFileName, &varStatus));
//	if (varStatus == VARIANT_TRUE)
//	{
//		CHK_HR(pXMLDom->get_xml(&bstrXML));
//		// XML DOM loaded from file
//		flgGoodToReturn = TRUE;
//	}
//	else
//	{
//		// Failed to load xml, get last parsing error
//		CHK_HR(pXMLDom->get_parseError(&pXMLErr));
//		CHK_HR(pXMLErr->get_reason(&bstrErr));
//		OutputDebugString(bstrErr);
//	}
//
//CleanUp:
//	SAFE_RELEASE(pXMLErr);
//	SysFreeString(bstrXML);
//	SysFreeString(bstrErr);
//	VariantClear(&varFileName);
//
//	if (flgGoodToReturn)
//	{
//		return pXMLDom;
//	}
//	else
//	{
//		SAFE_RELEASE(pXMLDom);
//		return NULL;
//	}
//}
//
//
//// Prototypes
//HRESULT ParseComments(XmlNodePtr _node);
//HRESULT ParseKeyboard(XmlNodePtr _kbNode, std::vector<KEYBOARD> * _vectorKeyboards);
//HRESULT ParseModifier(XmlNodePtr _modNode, KEYBOARD* _keyboard);
//HRESULT ParseLevel(XmlNodePtr _levelNode, KEYBOARD* _keyboard);
//HRESULT ParseUnicode(XmlNodePtr _unicodeNode, Level* _level);
//HRESULT ParseMacro(XmlNodePtr _macroNode, Level* _level);
//HRESULT ParseScript(XmlNodePtr _scriptNode, Level* _level);
//HRESULT ParseDeadkey(XmlNodePtr _keyNode, Level* _level);
//
//
//
//// Implementation of ParseSettings
//BOOL Multikeys::Remapper::ParseSettings(std::wstring filename)
//{
//	// Clear anything already in the remapper
//	this->vectorKeyboards.clear();
//
//	// Allocating memory
//	
//	// Whether this function has been successful; to use with CleanUp annotation
//	BOOL retVal = FALSE;
//	// Return value of functions
//	HRESULT hr = S_OK;		// 
//	// Root node for an xml file
//	XmlDocumentPtr xmlDomDocument = NULL;
//	// String for a tag
//	BSTR xmlTag_keyboard = SysAllocString(L"keyboard");
//	CHK_ALLOC(xmlTag_keyboard);
//	// List of child nodes
//	XmlNodeListPtr kbNodeList = NULL;
//	// Child node
//	XmlNodePtr kbNode = NULL;
//	BSTR selectionLanguage = SysAllocString(L"SelectionLanguage");
//	CHK_ALLOC(selectionLanguage);
//	VARIANT variant;
//	// CHK_HR: Macro that jumps to CleanUp if call fails
//	CHK_HR(CoInitializeEx(NULL, COINIT_MULTITHREADED));
//
//	// Load an xml file; xmlDomDocument is the root node.
//	xmlDomDocument = loadDom(filename.c_str());
//	if (xmlDomDocument == NULL) return FALSE;
//
//	// Remove all comments
//	VariantFromString(L"XPath", variant);
//	CHK_HR(xmlDomDocument->setProperty(selectionLanguage, variant));
//	CHK_HR(ParseComments(xmlDomDocument));
//
//		
//	// Load all descendant nodes with name "keyboard" into a node list
//	CHK_HR(xmlDomDocument->getElementsByTagName(xmlTag_keyboard, &kbNodeList));
//		
//	// Get length of node list
//	long length = 0;
//	CHK_HR(kbNodeList->get_length(&length));
//
//	for (long i = 0; i < length; i++)
//	{
//		// put keyboard node into a node pointer;
//		CHK_HR(kbNodeList->get_item(i, &kbNode));
//
//		// Call function to evaluate it, using this object's vector of keyboards
//		CHK_HR(ParseKeyboard(kbNode, &vectorKeyboards));
//	}
//
//	// retVal is only ever true if this line is reached
//	retVal = TRUE;
//CleanUp:
//	SysFreeString(selectionLanguage);
//	SAFE_RELEASE(kbNode);
//	SAFE_RELEASE(kbNodeList);
//	SAFE_RELEASE(xmlDomDocument);
//	SysFreeString(xmlTag_keyboard);
//	VariantClear(&variant);
//	CoUninitialize();
//	return retVal;
//}
//
//
//
//// Removes every comment under _node. May call with an XmlDOMDocument.
//// Set the document's selection language to "XPath" before calling this function.
//HRESULT ParseComments(XmlNodePtr _node)
//{
//	HRESULT hr = S_OK;
//	HRESULT retVal = E_FAIL;
//
//	XmlNodeListPtr pNodeList = NULL;
//	XmlNodePtr pNode = NULL;
//	XmlNodePtr pParentNode = NULL;
//	XmlNodePtr pDiscardNode = NULL;
//	long length = 0;
//	VARIANT_BOOL variantBool = 0;
//	BSTR query = SysAllocString(L"//comment()");
//	CHK_ALLOC(query);
//
//
//	CHK_HR(_node->selectNodes(query, &pNodeList));
//	CHK_HR(pNodeList->get_length(&length));
//	for (long i = 0; i < length; i++)
//	{
//		// get each item
//		CHK_HR(pNodeList->get_item(i, &pNode));
//
//		// get its parent and remove it
//		CHK_HR(pNode->get_parentNode(&pParentNode));
//		CHK_HR(pParentNode->removeChild(pNode, &pDiscardNode));
//
//	#if DEBUG
//		OutputDebugString(L"Parser: Removing a comment.\n");
//	#endif
//
//	}
//
//
//	retVal = S_OK;
//CleanUp:
//	SysFreeString(query);
//	SAFE_RELEASE(pNodeList);
//	SAFE_RELEASE(pNode);
//	SAFE_RELEASE(pParentNode);
//	SAFE_RELEASE(pDiscardNode);
//	return retVal;
//}
//
//
//
//// Reads a single IXMLDOMNode, puts its data into a KEYBOARD structure, and adds
////		it to the vector of keyboards
//HRESULT ParseKeyboard(XmlNodePtr _kbNode, std::vector<KEYBOARD> * _vectorKeyboards)
//{
//	// For use with CHK_HR
//	HRESULT hr = S_OK;
//	// Return value
//	HRESULT retVal = E_FAIL;
//	std::wstring wideString;
//	XmlNamedNodeMapPtr pNamedNodeMap = NULL;
//	// List of nodes
//	XmlNodeListPtr pNodeList = NULL;
//	// Pointer to one node
//	XmlNodePtr pNode = NULL;
//	// xml tag names
//	BSTR xmlTag_modifier = SysAllocString(L"modifier");
//	CHK_ALLOC(xmlTag_modifier);
//	BSTR xmlTag_level = SysAllocString(L"level");
//	CHK_ALLOC(xmlTag_level);
//	BSTR xmlAttribute_Name = SysAllocString(L"Name");
//	CHK_ALLOC(xmlAttribute_Name);
//	
//	// also work binary string
//	BSTR workBSTR;
//	// For looping through lists
//	long length = 0;
//	VARIANT_BOOL variantBool = 0;
//
//
//	// Instantiate a new KEYBOARD to fill with data
//	auto keyboard = new KEYBOARD();
//
//	// Get all attributes of this keyboard
//	CHK_HR(_kbNode->get_attributes(&pNamedNodeMap));
//	// Read Name:
//	CHK_HR(pNamedNodeMap->getNamedItem(xmlAttribute_Name, &pNode));
//	CHK_HR(pNode->get_text(&workBSTR));
//	// Put keyboard name in keyboard
//	keyboard->deviceName = std::wstring(workBSTR, SysStringLen(workBSTR));
//	// Keyboard node should also have an alias. That's just for the GUI,
//	// and we do not read it here.
//#if DEBUG
//	OutputDebugString((L"Parser: Reading keyboard " + keyboard->deviceName + L'\n').c_str());
//#endif
//	CHK_HR(pNode->get_text(&workBSTR));
//	wideString = std::wstring(workBSTR, SysStringLen(workBSTR));
//
//	// Get all child nodes:
//	CHK_HR(_kbNode->hasChildNodes(&variantBool));
//	if (variantBool == VARIANT_FALSE) { retVal = S_OK; goto CleanUp; }
//
//	CHK_HR(_kbNode->get_childNodes(&pNodeList));
//	CHK_HR(pNodeList->get_length(&length));
//	
//	// loop through node list, first looking for modifiers
//	for (long i = 0; i < length; i++)
//	{
//		CHK_HR(pNodeList->get_item(i, &pNode));
//		CHK_HR(pNode->get_nodeName(&workBSTR));
//		if (VarBstrCmp(workBSTR, xmlTag_modifier, 0, 0) == VARCMP_EQ)
//		{
//			ParseModifier(pNode, keyboard);
//		}
//	}
//
//	// Then again, looking for levels
//	for (long i = 0; i < length; i++)
//	{
//		CHK_HR(pNodeList->get_item(i, &pNode));
//		CHK_HR(pNode->get_nodeName(&workBSTR));
//		if (VarBstrCmp(workBSTR, xmlTag_level, 0, 0) == VARCMP_EQ)
//		{
//			ParseLevel(pNode, keyboard);
//		}
//	}
//
//	keyboard->resetModifierState();
//	_vectorKeyboards->push_back(*keyboard);
//
//
//	retVal = S_OK;
//
//CleanUp:
//	SAFE_RELEASE(pNamedNodeMap);
//	SAFE_RELEASE(pNodeList);
//	SAFE_RELEASE(pNode);
//	SysFreeString(xmlTag_modifier);
//	SysFreeString(xmlTag_level);
//	SysFreeString(xmlAttribute_Name);
//	return retVal;
//}
//
//
//
//// Receives a node that represents a modifier, and places that modifier in _keyboard.
//HRESULT ParseModifier(XmlNodePtr _modNode, KEYBOARD* _keyboard)
//{
//	HRESULT hr = S_OK;
//	HRESULT retValue = E_FAIL;
//
//	std::wstring wideString;
//	XmlNamedNodeMapPtr pNamedNodeMap = NULL;
//	XmlNodePtr pNode = NULL;
//	BSTR xmlTagVKey = SysAllocString(L"vKey");
//	CHK_ALLOC(&xmlTagVKey);
//	BSTR xmlTagName = SysAllocString(L"Name");
//	CHK_ALLOC(&xmlTagName);
//
//	BSTR workBSTR;
//	// Variants for retrieving data
//	VARIANT variant;
//
//	// Get the text value of this node
//	CHK_HR(_modNode->get_text(&workBSTR));
//	wideString = std::wstring(workBSTR, SysStringLen(workBSTR));
//#if DEBUG
//	OutputDebugString((L"Parser: Adding modifier " + wideString + L"\n").c_str());
//#endif
//	try {
//		_keyboard->addModifier(std::stoi(wideString, nullptr, 16), FALSE);
//	}
//	catch (std::exception ex) {
//		goto CleanUp;
//	}
//	
//	retValue = S_OK;
//CleanUp:
//	VariantClear(&variant);
//	SysFreeString(xmlTagVKey);
//	SysFreeString(xmlTagName);
//	SysFreeString(workBSTR);
//	SAFE_RELEASE(pNamedNodeMap);
//	SAFE_RELEASE(pNode);
//	return retValue;
//}
//
//
//
//// Code that splits a string, by the Stack Overflow Community
//template<typename Out>
//void split(const std::string &s, char delim, Out result) {
//	std::stringstream ss;
//	ss.str(s);
//	std::string item;
//	while (std::getline(ss, item, delim)) {
//		*(result++) = item;
//	}
//}
//std::vector<std::string> split(const std::string &s, char delim) {
//	std::vector<std::string> elems;
//	split(s, delim, std::back_inserter(elems));
//	return elems;
//}
//// Utility function for splitting a string to get hexadecimals
//USHORT* splitParseModifiers(std::wstring &s) {
//	std::string cvtd =
//		std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(s);
//
//	USHORT* states = new USHORT[8];
//	for (int i = 0; i < 8; i++) states[i] = 0;
//	std::vector<std::string> splstr = split(cvtd, ',');
//	if (splstr.size() > 8) return NULL;
//	for (int i = 0; i < splstr.size(); i++) {
//		try {
//			*(states + i) = (USHORT)std::stoi(splstr.at(i), nullptr, 16);
//		}
//		catch (std::exception ex) {
//			return nullptr;
//		}
//	}
//	return states;
//}
//// Utility function for getting a scancode
//// If there is a prefix (in the form E0: or E1:), it is bitshifted 16 bits left
////		and added to the scancode
//DWORD ParseScancode(std::wstring strScancode)
//{
//	std::string cvtd =
//		std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(strScancode);
//
//	std::vector<std::string> splstr = split(cvtd, ':');
//	if (splstr.size() == 1) {	// No prefix
//		return (DWORD)std::stoi(splstr.at(0), nullptr, 16);
//	}
//	else if (splstr.size() == 2) {	// Has prefix
//		DWORD retVal = (
//			((WORD)std::stoi(splstr.at(0), nullptr, 16) << 16)
//			| (WORD)std::stoi(splstr.at(1), nullptr, 16)
//			);
//		return retVal;
//	}
//	else return 0;
//}
//
//
//// Level can have <unicode>, <macro> and <script> nodes.
//HRESULT ParseLevel(XmlNodePtr _levelNode, KEYBOARD* _keyboard)
//{
//	HRESULT hr = S_OK;
//	HRESULT retVal = E_FAIL;
//
//	std::wstring wideString;
//	XmlNamedNodeMapPtr pNamedNodeMap = NULL;
//	XmlNodePtr pNode = NULL;
//	XmlNodePtr pNodeNext = NULL;
//	BSTR workBSTR;
//	BSTR xmlAttributeModifiers = SysAllocString(L"Modifiers");
//	CHK_ALLOC(xmlAttributeModifiers);
//	BSTR xmlTagUnicode = SysAllocString(L"unicode");
//	CHK_ALLOC(xmlTagUnicode);
//	BSTR xmlTagMacro = SysAllocString(L"macro");
//	CHK_ALLOC(xmlTagMacro);
//	BSTR xmlTagExecute = SysAllocString(L"execute");
//	CHK_ALLOC(xmlTagExecute);
//	BSTR xmlTagDeadKey = SysAllocString(L"deadkey");
//	CHK_ALLOC(xmlTagDeadKey);
//	
//	// Check its attribute Modifiers
//	CHK_HR(_levelNode->get_attributes(&pNamedNodeMap));
//	CHK_HR(pNamedNodeMap->getNamedItem(xmlAttributeModifiers, &pNode));
//	CHK_HR(pNode->get_text(&workBSTR));
//	wideString = std::wstring(workBSTR, SysStringLen(workBSTR));
//	USHORT* modParams = splitParseModifiers(wideString);
//	CHK_ALLOC(modParams);
//
//	Level* pLevel = new Level();
//	pLevel->setModifiers(
//		modParams[0], modParams[1],
//		modParams[2], modParams[3],
//		modParams[4], modParams[5],
//		modParams[6], modParams[7]);
//	if (DEBUG) OutputDebugString(L"Parser: Adding a level\n");
//
//	// Process each child
//	CHK_HR(_levelNode->get_firstChild(&pNode));
//	while (pNode != NULL)
//	{
//		// Get the name of this node
//		CHK_HR(pNode->get_nodeName(&workBSTR));
//		if (VarBstrCmp(workBSTR, xmlTagUnicode, 0, 0) == VARCMP_EQ)
//			ParseUnicode(pNode, pLevel);
//		else if (VarBstrCmp(workBSTR, xmlTagMacro, 0, 0) == VARCMP_EQ)
//			ParseMacro(pNode, pLevel);
//		else if (VarBstrCmp(workBSTR, xmlTagExecute, 0, 0) == VARCMP_EQ)
//			ParseScript(pNode, pLevel);
//		else if (VarBstrCmp(workBSTR, xmlTagDeadKey, 0, 0) == VARCMP_EQ)
//			ParseDeadkey(pNode, pLevel);
//		else goto CleanUp;
//
//		CHK_HR(pNode->get_nextSibling(&pNodeNext));
//		pNode = pNodeNext;
//	}
//
//
//	_keyboard->levels.push_back(*pLevel);
//	delete pLevel;
//
//	retVal = S_OK;
//CleanUp:
//	SysFreeString(workBSTR);
//	SysFreeString(xmlAttributeModifiers);
//	SysFreeString(xmlTagUnicode);
//	SysFreeString(xmlTagMacro);
//	SysFreeString(xmlTagExecute);
//	SysFreeString(xmlTagDeadKey);
//	SAFE_RELEASE(pNamedNodeMap);
//	SAFE_RELEASE(pNode);
//	SAFE_RELEASE(pNodeNext);
//	return retVal;
//}
//
//
//
//HRESULT ParseUnicode(XmlNodePtr _unicodeNode, Level* _level)
//{
//	HRESULT hr = S_OK;
//	HRESULT retVal = E_FAIL;
//
//	std::wstring wideString;
//	XmlNamedNodeMapPtr pNamedNodeMap = NULL;
//	XmlNodeListPtr pNodeList = NULL;
//	XmlNodePtr pNode = NULL;
//	BSTR workBSTR;
//	BSTR xmlAttributeScancode = SysAllocString(L"Scancode");
//	CHK_ALLOC(xmlAttributeScancode);
//	BSTR xmlAttributeTriggerOnRepeat = SysAllocString(L"TriggerOnRepeat");
//	CHK_ALLOC(xmlAttributeTriggerOnRepeat);
//	BSTR binaryStringTrue = SysAllocString(L"True");
//	CHK_ALLOC(binaryStringTrue);
//
//	BOOL triggerOnRepeat = FALSE;
//	DWORD scancode = 0;
//	UINT* codepoints = NULL;
//	long codepointCount = 0;
//	VARIANT_BOOL variantBool = 0;
//
//	// Retrieve scancode attribute
//	CHK_HR(_unicodeNode->get_attributes(&pNamedNodeMap));
//	CHK_HR(pNamedNodeMap->getNamedItem(xmlAttributeScancode, &pNode));
//	CHK_HR(pNode->get_text(&workBSTR));
//	wideString = std::wstring(workBSTR, SysStringLen(workBSTR));
//	try {
//		scancode = ParseScancode(wideString);
//	}
//	catch (std::exception ex) {
//		goto CleanUp;
//	}
//	// Retrieve triggerOnRepeat attribute
//	CHK_HR(pNamedNodeMap->getNamedItem(xmlAttributeTriggerOnRepeat, &pNode));
//	CHK_HR(pNode->get_text(&workBSTR));
//	if (VarBstrCmp(workBSTR, binaryStringTrue, 0, 0) == VARCMP_EQ) {
//		triggerOnRepeat = TRUE;
//	}
//
//	// Retrieve the number of scancodes, then read each scancode
//	CHK_HR(_unicodeNode->hasChildNodes(&variantBool));
//	if (variantBool == VARIANT_FALSE) goto CleanUp;
//	CHK_HR(_unicodeNode->get_childNodes(&pNodeList));
//	CHK_HR(pNodeList->get_length(&codepointCount));
//
//	codepoints = new UINT[codepointCount];
//	for (long i = 0; i < codepointCount; i++)
//	{
//		CHK_HR(pNodeList->get_item(i, &pNode));
//		CHK_HR(pNode->get_text(&workBSTR));
//		wideString = std::wstring(workBSTR, SysStringLen(workBSTR));
//		try {
//			codepoints[i] = std::stoi(wideString, nullptr, 16);
//		}
//		catch (std::exception ex) {
//			goto CleanUp;
//		}
//	}
//
//	auto pUnicodeCommand = new UnicodeCommand(codepoints, codepointCount, triggerOnRepeat);
//	_level->insertPair(scancode, pUnicodeCommand);
//#if DEBUG
//	OutputDebugString(L"Parser: Inserted a new unicode remap.\n");
//#endif
//	delete[] codepoints;
//
//	retVal = S_OK;
//CleanUp:
//	SysFreeString(xmlAttributeScancode);
//	SysFreeString(xmlAttributeTriggerOnRepeat);
//	SysFreeString(workBSTR);
//	SysFreeString(binaryStringTrue);
//	SAFE_RELEASE(pNamedNodeMap);
//	SAFE_RELEASE(pNodeList);
//	SAFE_RELEASE(pNode);
//
//	return retVal;
//}
//
//
//HRESULT ParseMacro(XmlNodePtr _macroNode, Level* _level)
//{
//	HRESULT hr = S_OK;
//	HRESULT retVal = E_FAIL;
//
//	std::wstring wideString;
//	XmlNamedNodeMapPtr pNamedNodeMap = NULL;
//	XmlNodeListPtr pNodeList = NULL;
//	XmlNodePtr pNode = NULL;
//	XmlNodePtr pNode_b = NULL;
//
//	BSTR workBSTR;
//	BSTR xmlAttributeScancode = SysAllocString(L"Scancode");
//	CHK_ALLOC(xmlAttributeScancode);
//	BSTR xmlAttributeTriggerOnRepeat = SysAllocString(L"TriggerOnRepeat");
//	CHK_ALLOC(xmlAttributeTriggerOnRepeat);
//	BSTR xmlAttributeKeyPress = SysAllocString(L"Keypress");
//	CHK_ALLOC(xmlAttributeTriggerOnRepeat);
//	BSTR binaryStringTrue = SysAllocString(L"True");
//	CHK_ALLOC(binaryStringTrue);
//	BSTR binaryStringUp = SysAllocString(L"Up");
//	CHK_ALLOC(binaryStringUp);
//
//	BOOL triggerOnRepeat = FALSE;
//	DWORD scancode = 0;
//	DWORD * keypresses = NULL;
//	long keypressCount = 0;
//	BOOL keypressUp = FALSE;
//	VARIANT_BOOL variantBool = 0;
//
//	// Retrieve scancode attribute
//	CHK_HR(_macroNode->get_attributes(&pNamedNodeMap));
//	CHK_HR(pNamedNodeMap->getNamedItem(xmlAttributeScancode, &pNode));
//	CHK_HR(pNode->get_text(&workBSTR));
//	wideString = std::wstring(workBSTR, SysStringLen(workBSTR));
//	try {
//		scancode = ParseScancode(wideString);
//	}
//	catch (std::exception ex) {
//		goto CleanUp;
//	}
//	// Retrieve triggerOnRepeat attribute
//	CHK_HR(pNamedNodeMap->getNamedItem(xmlAttributeTriggerOnRepeat, &pNode));
//	CHK_HR(pNode->get_text(&workBSTR));
//	if (VarBstrCmp(workBSTR, binaryStringTrue, 0, 0) == VARCMP_EQ) {
//		triggerOnRepeat = TRUE;
//	}
//
//	// Retrieve the number of keypresses, then read each keypress
//	CHK_HR(_macroNode->hasChildNodes(&variantBool));
//	if (variantBool == VARIANT_FALSE) goto CleanUp;
//	CHK_HR(_macroNode->get_childNodes(&pNodeList));
//	CHK_HR(pNodeList->get_length(&keypressCount));
//	keypresses = new DWORD[keypressCount];
//
//	for (long i = 0; i < keypressCount; i++)
//	{
//		// Get node:
//		CHK_HR(pNodeList->get_item(i, &pNode));
//		// Retrieve keypressed attribute:
//		CHK_HR(pNode->get_attributes(&pNamedNodeMap));
//		CHK_HR(pNamedNodeMap->getNamedItem(xmlAttributeKeyPress, &pNode_b));
//		CHK_HR(pNode_b->get_text(&workBSTR));
//		if (VarBstrCmp(workBSTR, binaryStringUp, 0, 0) == VARCMP_EQ) {
//			keypressUp = TRUE;
//		}
//		else keypressUp = FALSE;
//		// Get node's proper text:
//		CHK_HR(pNode->get_text(&workBSTR));
//		wideString = std::wstring(workBSTR, SysStringLen(workBSTR));
//		try {
//			keypresses[i] = std::stoi(wideString, nullptr, 16);
//		}
//		catch (std::exception) {
//			goto CleanUp;
//		}
//		if (keypressUp) keypresses[i] |= 0x80000000;
//	}
//
//	auto pMacro = new MacroCommand(keypresses, keypressCount, triggerOnRepeat);
//	_level->insertPair(scancode, pMacro);
//#if DEBUG
//	OutputDebugString(L"Parser: Inserted a new macro remap.\n");
//#endif
//	delete[] keypresses;
//
//	retVal = S_OK;
//CleanUp:
//	SysFreeString(xmlAttributeScancode);
//	SysFreeString(xmlAttributeTriggerOnRepeat);
//	SysFreeString(xmlAttributeKeyPress);
//	SysFreeString(workBSTR);
//	SysFreeString(binaryStringTrue);
//	SysFreeString(binaryStringUp);
//	SAFE_RELEASE(pNamedNodeMap);
//	SAFE_RELEASE(pNodeList);
//	SAFE_RELEASE(pNode);
//	SAFE_RELEASE(pNode_b);
//
//	return retVal;
//}
//
//
//HRESULT ParseScript(XmlNodePtr _scriptNode, Level* _level)
//{
//	HRESULT hr = S_OK;
//	HRESULT retVal = E_FAIL;
//
//	std::wstring wideString;
//	std::wstring wstrPath;
//	std::wstring wstrParameter;
//	XmlNamedNodeMapPtr pNamedNodeMap = NULL;
//	XmlNodePtr pNode = NULL;
//	XmlNodePtr pNodeNext = NULL;
//
//	BSTR workBSTR;
//	BSTR xmlAttributeScancode = SysAllocString(L"Scancode");
//	CHK_ALLOC(xmlAttributeScancode);
//	BSTR xmlTagPath = SysAllocString(L"path");
//	CHK_ALLOC(xmlTagPath);
//	BSTR xmlTagParameter = SysAllocString(L"parameter");
//	CHK_ALLOC(xmlTagParameter);
//
//	DWORD scancode = 0;
//	BOOL hasPath = FALSE;
//	BOOL hasParameter = FALSE;
//	VARIANT_BOOL variantBool = 0;
//
//	// Retrieve scancode attribute
//	CHK_HR(_scriptNode->get_attributes(&pNamedNodeMap));
//	CHK_HR(pNamedNodeMap->getNamedItem(xmlAttributeScancode, &pNode));
//	CHK_HR(pNode->get_text(&workBSTR));
//	wideString = std::wstring(workBSTR, SysStringLen(workBSTR));
//	try {
//		scancode = ParseScancode(wideString);
//	}
//	catch (std::exception ex) {
//		goto CleanUp;
//	}
//
//	// Retrieve children, look at path and parameter
//	CHK_HR(_scriptNode->hasChildNodes(&variantBool));
//	if (variantBool == VARIANT_FALSE) goto CleanUp;
//	CHK_HR(_scriptNode->get_firstChild(&pNode));
//	while (pNode != NULL)
//	{
//		// get the name of this node
//		CHK_HR(pNode->get_nodeName(&workBSTR));
//		if (wcscmp(workBSTR, xmlTagPath) == 0)
//		{
//			hasPath = TRUE;
//			CHK_HR(pNode->get_text(&workBSTR));	// <- careful - reusing variable
//			wstrPath = std::wstring(workBSTR, SysStringLen(workBSTR));
//		}
//		else if (wcscmp(workBSTR, xmlTagParameter) == 0)
//		{
//			hasParameter = TRUE;
//			CHK_HR(pNode->get_text(&workBSTR));
//			wstrParameter = std::wstring(workBSTR, SysStringLen(workBSTR));
//		}
//
//		CHK_HR(pNode->get_nextSibling(&pNodeNext));
//		pNode = pNodeNext;
//	}
//
//	if (hasPath)
//	{
//		ExecutableCommand* scrPointer;
//		if (hasParameter)
//		{
//			scrPointer = new ExecutableCommand(wstrPath, wstrParameter);
//		}
//		else
//		{
//			scrPointer = new ExecutableCommand(wstrPath);
//		}
//		_level->insertPair(scancode, scrPointer);
//#if DEBUG
//		OutputDebugString(L"Parser: Inserted a new executable remap.\n");
//#endif
//	}
//
//	retVal = S_OK;
//CleanUp:
//	SysFreeString(xmlAttributeScancode);
//	SysFreeString(xmlTagPath);
//	SysFreeString(xmlTagParameter);
//	SAFE_RELEASE(pNamedNodeMap);
//	SAFE_RELEASE(pNode);
//	SAFE_RELEASE(pNodeNext);
//
//	return retVal;
//}
//
//
//
//HRESULT ParseIndependentCodepoints(
//	XmlNodePtr _independentNode, UINT**codepoints, UINT*codepointCount);
//
//HRESULT ParseReplacement(XmlNodePtr _replacementNode,
//	UnicodeCommand** unicodeFrom, UnicodeCommand** unicodeTo);
//
//HRESULT ParseDeadkey(XmlNodePtr _keyNode, Level* _level)
//{
//	HRESULT hr = S_OK;
//	HRESULT retVal = E_FAIL;
//
//	XmlNamedNodeMapPtr pNamedNodeMap = NULL;
//	XmlNodeListPtr pNodeList = NULL;
//	XmlNodePtr pNode = NULL;
//
//	std::wstring wideString;
//	BSTR workBSTR;
//	BSTR xmlAttributeScancode = SysAllocString(L"Scancode");
//	CHK_ALLOC(xmlAttributeScancode);
//	BSTR xmlTagIndependent = SysAllocString(L"independent");
//	CHK_ALLOC(xmlTagIndependent);
//	BSTR xmlTagReplacement = SysAllocString(L"replacement");
//	CHK_ALLOC(xmlTagReplacement);
//	BSTR xmlTagFrom = SysAllocString(L"from");
//	CHK_ALLOC(xmlTagFrom);
//	BSTR xmlTagTo = SysAllocString(L"to");
//	CHK_ALLOC(xmlTagTo);
//	BSTR xmlTagCodepoint = SysAllocString(L"codepoint");
//	CHK_ALLOC(xmlTagCodepoint);
//
//	// Scancode of this dead key:
//	DWORD scancode = 0;
//	// Independent codepoints:
//	UINT* independents = NULL;
//	UINT independentCount = 0;
//	// Replacements:
//	UnicodeCommand** replacementsFrom = NULL;
//	UnicodeCommand** replacementsTo = NULL;
//	UINT replacementCount = 0;
//	// Work:
//	long length = 0;
//	VARIANT_BOOL variantBool = 0;
//
//
//	// First, read scancode
//	CHK_HR(_keyNode->get_attributes(&pNamedNodeMap));
//	CHK_HR(pNamedNodeMap->getNamedItem(xmlAttributeScancode, &pNode));
//	CHK_HR(pNode->get_text(&workBSTR));
//	wideString = std::wstring(workBSTR, SysStringLen(workBSTR));
//	try {
//		scancode = ParseScancode(wideString);
//	}
//	catch (std::exception ex) {
//		goto CleanUp;
//	}
//
//	// Get all children nodes, count how many replacements there are
//	CHK_HR(_keyNode->hasChildNodes(&variantBool));
//	if (variantBool == VARIANT_FALSE) {
//		goto CleanUp;
//	}
//	CHK_HR(_keyNode->get_childNodes(&pNodeList));
//	CHK_HR(pNodeList->get_length(&length));
//	for (long i = 0; i < length; i++)
//	{
//		CHK_HR(pNodeList->get_item(i, &pNode));
//		CHK_HR(pNode->get_nodeName(&workBSTR));
//		if (VarBstrCmp(workBSTR, xmlTagReplacement, 0, 0) == VARCMP_EQ)
//		{
//			replacementCount++;
//		}
//
//		// Also, if we find the independent node, parse it now
//		if (VarBstrCmp(workBSTR, xmlTagIndependent, 0, 0) == VARCMP_EQ)
//		{
//			CHK_HR(ParseIndependentCodepoints(pNode, &independents, &independentCount));
//		}
//	}
//
//	// ReplacementCount contains the amount of replacements
//	replacementsFrom = new UnicodeCommand*[replacementCount];
//	replacementsTo = new UnicodeCommand*[replacementCount];
//
//	// Go through list again, this time initializing each element
//	for (long i = 0, currentIndex = 0; i < length; i++)
//	{
//		CHK_HR(pNodeList->get_item(i, &pNode));
//		CHK_HR(pNode->get_nodeName(&workBSTR));
//		if (VarBstrCmp(workBSTR, xmlTagReplacement, 0, 0) == VARCMP_EQ)
//		{
//			// If this is replacement node, use it to initialize an element
//			ParseReplacement(pNode, &(replacementsFrom[currentIndex]), &(replacementsTo[currentIndex]));
//			// Move on
//			currentIndex++;
//		}
//	}
//
//	// All elements are initialized to actual instances of UnicodeCommand
//	auto deadKeyPointer = new DeadKeyCommand(independents, independentCount,
//		replacementsFrom, replacementsTo, replacementCount);
//	_level->insertPair(scancode, deadKeyPointer);
//	// Everything stays in memory
//
//
//	retVal = S_OK;
//CleanUp:
//	SysFreeString(xmlAttributeScancode);
//	SysFreeString(xmlTagIndependent);
//	SysFreeString(xmlTagReplacement);
//	SysFreeString(xmlTagFrom);
//	SysFreeString(xmlTagTo);
//	SysFreeString(xmlTagCodepoint);
//	SAFE_RELEASE(pNamedNodeMap);
//	SAFE_RELEASE(pNodeList);
//	SAFE_RELEASE(pNode);
//	return retVal;
//}
//
//HRESULT ParseIndependentCodepoints(
//	XmlNodePtr _independentNode, UINT**codepoints, UINT*codepointCount)
//{
//	HRESULT hr = S_OK;
//	HRESULT retVal = E_FAIL;
//
//	XmlNodeListPtr pNodeList = NULL;
//	XmlNodePtr pNode = NULL;
//
//	std::wstring wideString;
//	BSTR workBSTR;
//	BSTR xmlTagCodepoint = SysAllocString(L"codepoint");
//	long length = 0;
//
//	CHK_HR(_independentNode->get_childNodes(&pNodeList));
//	CHK_HR(pNodeList->get_length(&length));
//	*codepointCount = (UINT)length;
//	*codepoints = new UINT[length];
//	for (long i = 0; i < length; i++)
//	{
//		CHK_HR(pNodeList->get_item(i, &pNode));
//		CHK_HR(pNode->get_nodeName(&workBSTR));
//		if (VarBstrCmp(workBSTR, xmlTagCodepoint, 0, 0) != VARCMP_EQ)
//			goto CleanUp;
//		CHK_HR(pNode->get_text(&workBSTR));
//		try {
//			wideString = std::wstring(workBSTR, SysStringLen(workBSTR));
//			(*codepoints)[i] = std::stoi(wideString.c_str(), nullptr, 16);
//		}
//		catch (std::exception ex) {
//			goto CleanUp;
//		}
//	}
//
//	retVal = S_OK;
//CleanUp:
//	SysFreeString(xmlTagCodepoint);
//	SysFreeString(workBSTR);
//	SAFE_RELEASE(pNodeList);
//	SAFE_RELEASE(pNode);
//	return retVal;
//}
//
//HRESULT ParseReplacement(XmlNodePtr _replacementNode,
//	UnicodeCommand** unicodeFrom, UnicodeCommand** unicodeTo)
//{
//	HRESULT hr = S_OK;
//	HRESULT retVal = E_FAIL;
//
//	std::wstring wideString;
//	XmlNodeListPtr pNodeList = NULL;
//	XmlNodePtr pNode = NULL;
//
//	BSTR workBSTR;
//	BSTR xmlTagFrom = SysAllocString(L"from");
//	CHK_ALLOC(xmlTagFrom);
//	BSTR xmlTagTo = SysAllocString(L"to");
//	CHK_ALLOC(xmlTagTo);
//	BSTR xmlTagCodepoint = SysAllocString(L"codepoint");
//	CHK_ALLOC(xmlTagCodepoint);
//
//	long length = 0;
//	BOOL replFromOk = FALSE;
//	BOOL replToOk = FALSE;
//	VARIANT_BOOL variantBool = 0;
//	
//	CHK_HR(_replacementNode->hasChildNodes(&variantBool));
//	if (!variantBool) goto CleanUp;
//	CHK_HR(_replacementNode->get_childNodes(&pNodeList));
//	CHK_HR(pNodeList->get_length(&length));
//	for (long i = 0; i < length; i++)
//	{
//		CHK_HR(pNodeList->get_item(i, &pNode));
//		CHK_HR(pNode->get_nodeName(&workBSTR));
//		if (VarBstrCmp(workBSTR, xmlTagFrom, 0, 0) == VARCMP_EQ)
//		{
//			replFromOk = TRUE;
//			UINT * codepoints;
//			UINT codepointCount = 0;
//			ParseIndependentCodepoints(pNode, &codepoints, &codepointCount);
//			(*unicodeFrom) = new UnicodeCommand(codepoints, codepointCount, true);
//			delete[] codepoints;
//		}
//		else if (VarBstrCmp(workBSTR, xmlTagTo, 0, 0) == VARCMP_EQ)
//		{
//			replToOk = TRUE;
//			UINT * codepoints;
//			UINT codepointCount = 0;
//			ParseIndependentCodepoints(pNode, &codepoints, &codepointCount);
//			(*unicodeTo) = new UnicodeCommand(codepoints, codepointCount, true);
//			delete[] codepoints;
//		}
//		else goto CleanUp;
//	}
//
//
//	if (replFromOk && replToOk) retVal = S_OK;
//CleanUp:
//	SysFreeString(workBSTR);
//	SysFreeString(xmlTagCodepoint);
//	SAFE_RELEASE(pNodeList);
//	SAFE_RELEASE(pNode);
//	return retVal;
//}
