#pragma once

namespace cpplab {
	enum class FontStyle {
		Arial, Calibri, Trebuchet
	};
	static const wchar_t* FontStyleToString(FontStyle style) {
		switch (style)
		{
		case cpplab::FontStyle::Arial:
			return L"Arial";
			break;
		case cpplab::FontStyle::Calibri:
			return L"Calibri";
			break;
		case cpplab::FontStyle::Trebuchet:
			return L"Trebuchet";
			break;
		default:
			return L"Arial"; //default font arial if switch misses
			break;
		}
	}
}