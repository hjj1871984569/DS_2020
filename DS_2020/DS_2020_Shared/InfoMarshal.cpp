#include "pch.h"
#include "InfoMarshal.h"
#include "xmlhelper.h"
#include "CommUtils.h"
wchar_t* InfoMarshal::Marshal(Info &inobj)
{
	CComPtr<IStream> pStream;
	CComPtr<IXmlWriter> pWriter;
	::CreateXmlWriter(__uuidof(IXmlWriter),
		reinterpret_cast<void**>(&pWriter),
		0);
	::CreateStreamOnHGlobal(0, TRUE, &pStream);
	CComPtr<IXmlWriterOutput> pOutput;
	CreateXmlWriterOutputWithEncodingName(pStream, nullptr, L"utf-16", &pOutput);
	pWriter->SetOutput(pOutput);
	//pWriter->SetProperty(XmlWriterProperty_Indent, TRUE);

	MYSTR clsid = inobj.GetClsid();
	pWriter->WriteStartElement(0, clsid, 0);
	std::map<MYSTR, std::vector<MYSTR>> props = inobj.GetProperties();
	for (auto i : props) {
		for (auto j : i.second) {
			pWriter->WriteStartElement(0, i.first, 0);
			pWriter->WriteString(j);
			pWriter->WriteEndElement();
		}
	}
	pWriter->WriteEndElement();
	pWriter->Flush();
	STATSTG result;
	pStream->Stat(&result, STATFLAG_DEFAULT);
	ULONG readret;
	WCHAR* pv = new WCHAR[result.cbSize.QuadPart + 1]{ 0 };
	LARGE_INTEGER move;
	move.QuadPart = 0;
	pStream->Seek(move, STREAM_SEEK_SET, NULL);
	pStream->Read(pv, result.cbSize.QuadPart, &readret);
	return pv;
}

Info InfoMarshal::Unmarshal(MYSTR xmlcode, DWORD flag)
{
	std::vector<MYSTR>* parseInfo = new std::vector<MYSTR>;
	if (flag & XMLPARSETYPE_article) {
		parseInfo->push_back(MYSTR(L"article"));
	}
	if (flag & XMLPARSETYPE_book) {
		parseInfo->push_back(MYSTR(L"book"));
	}
	if (flag & XMLPARSETYPE_incollection) {
		parseInfo->push_back(MYSTR(L"incollection"));
	}
	if (flag & XMLPARSETYPE_inproceedings) {
		parseInfo->push_back(MYSTR(L"inproceedings"));
	}
	if (flag & XMLPARSETYPE_mastersthesis) {
		parseInfo->push_back(MYSTR(L"mastersthesis"));
	}
	if (flag & XMLPARSETYPE_phdthesis) {
		parseInfo->push_back(MYSTR(L"phdthesis"));
	}
	if (flag & XMLPARSETYPE_proceedings) {
		parseInfo->push_back(MYSTR(L"proceedings"));
	}
	if (flag & XMLPARSETYPE_www) {
		parseInfo->push_back(MYSTR(L"www"));
	}

	CComPtr<IStream> pStream;
	CComPtr<IXmlReader> m_pReader;
	CComPtr<IXmlReaderInput> pInput;
	HRESULT hr = ::CreateStreamOnHGlobal(0, TRUE, &pStream);
	CreateXmlReaderInputWithEncodingName(pStream, nullptr, L"utf-16", false, nullptr, &pInput);

	ULONG written;
	wchar_t* pv = (wchar_t*)xmlcode;
	pStream->Write(pv, 2 * wcslen(pv) + 1, &written);
	LARGE_INTEGER move;
	move.QuadPart = 0;
	pStream->Seek(move, STREAM_SEEK_SET, NULL);
	CreateXmlReader(__uuidof(IXmlReader), (void**)&m_pReader, NULL);
	m_pReader->SetInput(pStream);
	m_pReader->SetProperty(XmlReaderProperty_DtdProcessing, TRUE);

	LPCWSTR szValue = NULL, curSection = NULL, localName = NULL;
	XmlNodeType nodeType;
	Info temp;

	while (S_OK == (hr = m_pReader->Read(&nodeType))) {
		if (nodeType == XmlNodeType_Element) {
			m_pReader->GetLocalName(&localName, NULL);

			// 解析类型
			std::vector<MYSTR>::iterator ret;
			ret = std::find(parseInfo->begin(), parseInfo->end(), MYSTR(localName));
			if (ret == parseInfo->end()) {
				continue;
			}

			// 看成是进入一个section
			curSection = localName;
			temp.SetClsid(MYSTR(curSection));

			if (S_OK == m_pReader->MoveToFirstAttribute()) {
				m_pReader->GetLocalName(&localName, NULL);
				m_pReader->GetValue(&szValue, NULL);
				temp.AddProperty(MYSTR(localName), MYSTR(szValue));
				while (S_OK == (hr = m_pReader->MoveToNextAttribute())) {
					m_pReader->GetLocalName(&localName, NULL);
					m_pReader->GetValue(&szValue, NULL);
					temp.AddProperty(MYSTR(localName), MYSTR(szValue));
				}
				m_pReader->MoveToElement();
			}

			while (lstrcmpW(localName, curSection) || nodeType != XmlNodeType_EndElement) {
				m_pReader->Read(&nodeType);
				m_pReader->GetLocalName(&localName, NULL);
				if (nodeType == XmlNodeType_Element) {
					if (!lstrcmpW(localName, curSection))
						break;
					while (nodeType != XmlNodeType_Text) {
						m_pReader->Read(&nodeType);
					}
					m_pReader->GetValue(&szValue, NULL);
					temp.AddProperty(MYSTR(localName), MYSTR(szValue));
					while (nodeType != XmlNodeType_EndElement) {
						m_pReader->Read(&nodeType);
					}
				}
			}
			// 假如没有title的, 将会被忽略
			if (temp.GetProperty(L"title").size()) {
				break;
			}
		}
	}
	delete parseInfo;
	return temp;
}

wchar_t* InfoMarshal::Marshal2v(std::vector<Info>&inobjs)
{
	MYSTR result;
	wchar_t* tmp;
	for (auto& i : inobjs) {
		tmp = Marshal(i);
		result += tmp;
		delete[] tmp;
	}
	size_t size = (size_t)result.length();
	WCHAR* pv = new WCHAR[size+2]{0};
	wcscpy(pv, (wchar_t*)result);
	return pv;
}

std::vector<Info> InfoMarshal::Unmarshal2v(MYSTR xmlcode, DWORD flag)
{
	std::vector<MYSTR>* parseInfo = new std::vector<MYSTR>;
	if (flag & XMLPARSETYPE_article) {
		parseInfo->push_back(MYSTR(L"article"));
	}
	if (flag & XMLPARSETYPE_book) {
		parseInfo->push_back(MYSTR(L"book"));
	}
	if (flag & XMLPARSETYPE_incollection) {
		parseInfo->push_back(MYSTR(L"incollection"));
	}
	if (flag & XMLPARSETYPE_inproceedings) {
		parseInfo->push_back(MYSTR(L"inproceedings"));
	}
	if (flag & XMLPARSETYPE_mastersthesis) {
		parseInfo->push_back(MYSTR(L"mastersthesis"));
	}
	if (flag & XMLPARSETYPE_phdthesis) {
		parseInfo->push_back(MYSTR(L"phdthesis"));
	}
	if (flag & XMLPARSETYPE_proceedings) {
		parseInfo->push_back(MYSTR(L"proceedings"));
	}
	if (flag & XMLPARSETYPE_www) {
		parseInfo->push_back(MYSTR(L"www"));
	}

	std::vector<Info> result;
	CComPtr<IStream> pStream;
	CComPtr<IXmlReader> m_pReader;
	CComPtr<IXmlReaderInput> pInput;
	HRESULT hr = ::CreateStreamOnHGlobal(0, TRUE, &pStream);
	CreateXmlReaderInputWithEncodingName(pStream, nullptr, L"utf-16", false, nullptr, &pInput);

	ULONG written;
	MYSTR realxml = L"<tagroot>";
	realxml += xmlcode;
	realxml += L"</tagroot>";
	wchar_t* pv = (wchar_t*)realxml;
	pStream->Write(pv, 2 * wcslen(pv) + 1, &written);

	LARGE_INTEGER move;
	move.QuadPart = 0;
	pStream->Seek(move, STREAM_SEEK_SET, NULL);
	CreateXmlReader(__uuidof(IXmlReader), (void**)&m_pReader, NULL);
	m_pReader->SetInput(pStream);
	m_pReader->SetProperty(XmlReaderProperty_DtdProcessing, TRUE);

	LPCWSTR szValue = NULL, curSection = NULL, localName = NULL;
	XmlNodeType nodeType;

	while (S_OK == (hr = m_pReader->Read(&nodeType))) {
		Info temp;
		if (nodeType == XmlNodeType_Element) {
			m_pReader->GetLocalName(&localName, NULL);

			std::vector<MYSTR>::iterator ret;
			ret = std::find(parseInfo->begin(), parseInfo->end(), MYSTR(localName));
			if (ret == parseInfo->end()) {
				continue;
			}

			curSection = localName;
			temp.SetClsid(MYSTR(curSection));

			if (S_OK == m_pReader->MoveToFirstAttribute()) {
				m_pReader->GetLocalName(&localName, NULL);
				m_pReader->GetValue(&szValue, NULL);
				temp.AddProperty(MYSTR(localName), MYSTR(szValue));
				while (S_OK == (hr = m_pReader->MoveToNextAttribute())) {
					m_pReader->GetLocalName(&localName, NULL);
					m_pReader->GetValue(&szValue, NULL);
					temp.AddProperty(MYSTR(localName), MYSTR(szValue));
				}
				m_pReader->MoveToElement();
			}

			while (lstrcmpW(localName, curSection) || nodeType != XmlNodeType_EndElement) {
				m_pReader->Read(&nodeType);
				m_pReader->GetLocalName(&localName, NULL);
				if (nodeType == XmlNodeType_Element) {
					if (!lstrcmpW(localName, curSection))
						break;
					while (nodeType != XmlNodeType_Text) {
						m_pReader->Read(&nodeType);
					}
					m_pReader->GetValue(&szValue, NULL);
					temp.AddProperty(MYSTR(localName), MYSTR(szValue));
					while (nodeType != XmlNodeType_EndElement) {
						m_pReader->Read(&nodeType);
					}
				}
			}
			if (temp.GetProperty(L"title").size()) {
				result.push_back(temp);
			}
		}
	}
	delete parseInfo;
	return result;
}
