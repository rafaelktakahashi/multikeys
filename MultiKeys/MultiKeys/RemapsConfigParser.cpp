// Implementation of BOOL Multikeys::Remapper::ParseSettings
// Uses MSXML to read an xml file with remapping configurations

// If anyone knows how to parse xml files with more elegance,
//		do try and replace this entire thing with a better
//		implementation. For now, hell will have to do.

#include "stdafx.h"
#include "Remaps.h"
// #include <comutil.h>
// #include <MsXml6.h>
#import "msxml6.dll" raw_interfaces_only
// Using #import rather than #include
// Microsoft doesn't recommend it, apparently. I care little.
#include <string.h>	// for splitting strings
#include <string>	// for parsing


// using namespace MSXML2;		// <- causes ambiguity.


// Macro that calls a COM method and jumps to CleanUp if it fails
#define CHK_HR(stmt)	do { hr=(stmt); if (FAILED(hr)) goto CleanUp; } while (0)

// Macro to verify memory allocation; jumps to CleanUp if out of memory and places result in hr
#define CHK_ALLOC(p)	do { if (!(p)) { hr = E_OUTOFMEMORY; goto CleanUp; } } while (0)

// Macro that releases a COM object if it's not already NULL
#define SAFE_RELEASE(p)	do { if ((p)) { (p)->Release(); (p) = NULL; } } while (0)


// Typedefs for MSXML namespace
typedef MSXML2::IXMLDOMDocument3		XmlDocument,		*XmlDocumentPtr;
typedef MSXML2::IXMLDOMNode				XmlNode,			*XmlNodePtr;
typedef MSXML2::IXMLDOMNodeList			XmlNodeList,		*XmlNodeListPtr;
typedef MSXML2::IXMLDOMAttribute		XmlAttribute,		*XmlAttributePtr;
typedef MSXML2::IXMLDOMNamedNodeMap		XmlNamedNodeMap,	*XmlNamedNodeMapPtr;
typedef MSXML2::IXMLDOMElement			XmlElement,			*XmlElementPtr;
typedef MSXML2::IXMLDOMParseError		XmlParseError,		*XmlParseErrorPtr;
typedef MSXML2::DOMDocument60			Document60;


// Helper function to create a VT_BSTR variant from a null terminated string (PCWSTR = WCHAR*)
HRESULT VariantFromString(PCWSTR wszValue, VARIANT &Variant)
{
	HRESULT hr = S_OK;
	BSTR bstr = SysAllocString(wszValue);
	CHK_ALLOC(bstr);
	V_VT(&Variant) = VT_BSTR;
	V_BSTR(&Variant) = bstr;

CleanUp:
	return hr;
}

// Helper function to create a DOM instance for a document
HRESULT CreateAndInitDOM(XmlDocumentPtr * ppDoc)
{
	HRESULT hr = CoCreateInstance(__uuidof(Document60), NULL,
		CLSCTX_INPROC_SERVER, IID_PPV_ARGS(ppDoc));
	if (SUCCEEDED(hr))
	{
		// these methods should not fail, so don't inspect result
		(*ppDoc)->put_async(VARIANT_FALSE);
		(*ppDoc)->put_validateOnParse(VARIANT_FALSE);
		(*ppDoc)->put_resolveExternals(VARIANT_FALSE);
	}
	return hr;
}

// Function that loads a document object model
// If a DOM is returned, call release on it when done
XmlDocumentPtr loadDom(std::wstring file)
{
	HRESULT hr = S_OK;
	XmlDocumentPtr pXMLDom = NULL;
	XmlParseErrorPtr pXMLErr = NULL;

	BOOL flgGoodToReturn = FALSE;

	BSTR bstrXML = NULL;
	BSTR bstrErr = NULL;
	VARIANT_BOOL varStatus;
	VARIANT varFileName;
	VariantInit(&varFileName);

	CHK_HR(CreateAndInitDOM(&pXMLDom));

	// CML file name to load
	CHK_HR(VariantFromString(file.c_str(), varFileName));
	CHK_HR(pXMLDom->load(varFileName, &varStatus));
	if (varStatus == VARIANT_TRUE)
	{
		CHK_HR(pXMLDom->get_xml(&bstrXML));
		// XML DOM loaded from file
		flgGoodToReturn = TRUE;
	}
	else
	{
		// Failed to load xml, get last parsing error
		CHK_HR(pXMLDom->get_parseError(&pXMLErr));
		CHK_HR(pXMLErr->get_reason(&bstrErr));
		OutputDebugString(bstrErr);
	}

CleanUp:
	SAFE_RELEASE(pXMLErr);
	SysFreeString(bstrXML);
	SysFreeString(bstrErr);
	VariantClear(&varFileName);

	if (flgGoodToReturn)
	{
		return pXMLDom;
	}
	else
	{
		SAFE_RELEASE(pXMLDom);
		return NULL;
	}
}


// Prototypes
HRESULT ParseKeyboard(XmlNodePtr _kbNode, std::vector<KEYBOARD> * _vectorKeyboards);
HRESULT ParseModifier(XmlNodePtr _modNode, KEYBOARD* _keyboard);
HRESULT ParseLevel(XmlNodePtr _levelNode, KEYBOARD* _keyboard);
HRESULT ParseUnicode(XmlNodePtr _unicodeNode, Level* _level);
HRESULT ParseMacro(XmlNodePtr _macroNode, Level* _level);
HRESULT ParseScript(XmlNodePtr _scriptNode, Level* _level);





// Implementation of ParseSettings
BOOL Multikeys::Remapper::ParseSettings(std::wstring filename)
{
	// Allocating memory
	
	// Whether this function has been successful; to use with CleanUp annotation
	BOOL retVal = FALSE;
	// Return value of functions
	HRESULT hr = S_OK;		// 
	// Root node for an xml file
	XmlDocumentPtr xmlDomDocument = NULL;
	// String for a tag
	BSTR xmlTag_keyboard = SysAllocString(L"keyboard");
	CHK_ALLOC(xmlTag_keyboard);
	// List of child nodes
	XmlNodeListPtr kbNodeList = NULL;
	// Child node
	XmlNodePtr kbNode = NULL;
	// CHK_HR: Macro that jumps to CleanUp if call fails
	CHK_HR(CoInitializeEx(NULL, COINIT_MULTITHREADED));

	// Load an xml file; xmlDomDocument is the root node.
	xmlDomDocument = loadDom(filename.c_str());
	if (xmlDomDocument == NULL) return FALSE;

		
	// Load all child nodes with name "keyboard" into a node list
	CHK_HR(xmlDomDocument->getElementsByTagName(xmlTag_keyboard, &kbNodeList));
		
	// Get length of node list
	long length = 0;
	CHK_HR(kbNodeList->get_length(&length));

	for (long i = 0; i < length; i++)
	{
		// put keyboard node into a node pointer;
		CHK_HR(kbNodeList->get_item(i, &kbNode));

		// Call function to evaluate it, using this object's vector of keyboards
		CHK_HR(ParseKeyboard(kbNode, &vectorKeyboards));
	}

	// retVal is only ever true if this line is reached
	retVal = TRUE;

CleanUp:
	SAFE_RELEASE(kbNode);
	SAFE_RELEASE(kbNodeList);
	SAFE_RELEASE(xmlDomDocument);
	SysFreeString(xmlTag_keyboard);
	CoUninitialize();
	return retVal;
}









// Reads a single IXMLDOMNode, puts its data into a KEYBOARD structure, and adds
//		it to the vector of keyboards
HRESULT ParseKeyboard(XmlNodePtr _kbNode, std::vector<KEYBOARD> * _vectorKeyboards)
{
	// For use with CHK_HR
	HRESULT hr = S_OK;
	// work binary string
	_bstr_t binaryString = new _bstr_t();
	// Return value
	HRESULT retVal = E_FAIL;
	XmlNamedNodeMapPtr pNamedNodeMap = NULL;
	// List of nodes
	XmlNodeListPtr pNodeList = NULL;
	// Pointer to one node
	XmlNodePtr pNode = NULL;
	// xml tag names
	BSTR xmlTag_modifier = SysAllocString(L"modifier");
	CHK_ALLOC(xmlTag_modifier);
	BSTR xmlTag_level = SysAllocString(L"level");
	CHK_ALLOC(xmlTag_level);
	BSTR xmlAttribute_Name = SysAllocString(L"Name");
	CHK_ALLOC(xmlAttribute_Name);
	BSTR xmlAttribute_Alias = SysAllocString(L"Alias");
	CHK_ALLOC(xmlAttribute_Alias);
	
	CHK_ALLOC(binaryString);
	// also work binary string
	BSTR workBSTR;
	// For looping through lists
	long length = 0;
	// Variant to retrieve data from COM methods
	VARIANT variant;
	// Variant boolean
	VARIANT_BOOL variantBool;



	// Instantiate a new KEYBOARD to fill with data
	auto keyboard = new KEYBOARD();

	// Get all attributes of this keyboard
	CHK_HR(_kbNode->get_attributes(&pNamedNodeMap));
	// Read Name:
	CHK_HR(pNamedNodeMap->getNamedItem(xmlAttribute_Name, &pNode));
	CHK_HR(pNode->get_nodeValue(&variant));
	binaryString = variant.bstrVal;
	CHK_ALLOC(binaryString);
	// Put keyboard name in keyboard
	keyboard->deviceName = std::wstring(binaryString, SysStringLen(binaryString));
	// Keyboard node should also have an alias. That's just for the GUI,
	// and we do not read it here.
	OutputDebugString((L"Parser: Reading keyboard " + keyboard->deviceName + L'\n').c_str());

	// Get all child nodes:
	CHK_HR(_kbNode->hasChildNodes(&variantBool));
	if (!variantBool) { retVal = S_OK; goto CleanUp; }

	CHK_HR(_kbNode->get_childNodes(&pNodeList));
	CHK_HR(pNodeList->get_length(&length));
	
	// loop through node list, first looking for modifiers
	for (long i = 0; i < length; i++)
	{
		CHK_HR(pNodeList->get_item(i, &pNode));
		CHK_HR(pNode->get_nodeName(&workBSTR));
		if (wcscmp(workBSTR, xmlTag_modifier) == 0)
		{
			ParseModifier(pNode, keyboard);
		}
	}

	// Then again, looking for levels
	for (long i = 0; i < length; i++)
	{
		CHK_HR(pNodeList->get_item(i, &pNode));
		CHK_HR(pNode->get_nodeName(&workBSTR));
		if (wcscmp(workBSTR, xmlTag_level) == 0)
		{
			ParseLevel(pNode, keyboard);
		}
	}

	keyboard->resetModifierState();
	_vectorKeyboards->push_back(*keyboard);




	retVal = S_OK;

CleanUp:
	SAFE_RELEASE(pNodeList);
	SAFE_RELEASE(pNode);
	SysFreeString(xmlTag_modifier);
	SysFreeString(xmlTag_level);
	SysFreeString(xmlAttribute_Name);
	SysFreeString(xmlAttribute_Alias);
	return retVal;
}





// Receives a node that represents a modifier, and places that modifier in _keyboard.
HRESULT ParseModifier(XmlNodePtr _modNode, KEYBOARD* _keyboard)
{
	HRESULT hr = S_OK;
	HRESULT retValue = E_FAIL;

	std::wstring wideString;
	XmlNamedNodeMapPtr pNamedNodeMap = NULL;
	XmlNodePtr pNode = NULL;
	BSTR xmlTagVKey = SysAllocString(L"vKey");
	CHK_ALLOC(&xmlTagVKey);
	BSTR xmlTagName = SysAllocString(L"Name");
	CHK_ALLOC(&xmlTagName);

	BSTR workBSTR;
	// Variants for retrieving data
	VARIANT variant;
	VARIANT_BOOL variantBool;

	// Get the text value of this node
	CHK_HR(_modNode->get_text(&workBSTR));
	wideString = std::wstring(workBSTR, SysStringLen(workBSTR));
	OutputDebugString((L"Parser: Adding modifier " + wideString + L"\n").c_str());
	try {
		_keyboard->addModifier(std::stoi(wideString, nullptr, 16), FALSE);
	}
	catch (std::exception ex) {
		goto CleanUp;
	}
	
	retValue = S_OK;
CleanUp:
	SysFreeString(xmlTagVKey);
	SysFreeString(xmlTagName);
	SysFreeString(workBSTR);
	SAFE_RELEASE(pNamedNodeMap);
	SAFE_RELEASE(pNode);
	return retValue;
}




// Level can have <unicode>, <macro> and <script> nodes.
HRESULT ParseLevel(XmlNodePtr _levelNode, KEYBOARD* _keyboard)
{
	HRESULT hr = S_OK;
	HRESULT retVal = E_FAIL;

	std::wstring wideString;
	XmlNamedNodeMapPtr pNamedNodeMap = NULL;
	XmlNodePtr pNode = NULL;
	BSTR workBSTR;
	BSTR xmlAttributeModifiers = SysAllocString(L"Modifiers");
	CHK_ALLOC(xmlAttributeModifiers);
	BSTR xmlTagUnicode = SysAllocString(L"Unicode");
	CHK_ALLOC(xmlTagUnicode);
	BSTR xmlTagMacro = SysAllocString(L"Macro");
	CHK_ALLOC(xmlTagMacro);
	BSTR xmlTagScript = SysAllocString(L"Script");
	CHK_ALLOC(xmlTagScript);
	
	// Check its attribute Modifiers
	CHK_HR(_levelNode->get_attributes(&pNamedNodeMap));
	CHK_HR(pNamedNodeMap->getNamedItem(xmlAttributeModifiers, &pNode));
	CHK_HR(pNode->get_text(&workBSTR));
	wideString = std::wstring(workBSTR, SysStringLen(workBSTR));
	// Now, I must split a string using C code.
	// O, what ungodly task has befallen me

	retVal = S_OK;
CleanUp:
	SysFreeString(xmlAttributeModifiers);
	SysFreeString(xmlTagUnicode);
	SysFreeString(xmlTagMacro);
	SysFreeString(xmlTagScript);
	SAFE_RELEASE(pNamedNodeMap);
	SAFE_RELEASE(pNode);
	return retVal;
}