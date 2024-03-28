#pragma once

namespace cpplab {
	enum class FontStyle {
		Arial, Calibri
	};
	static const wchar_t* FontStyleToString(FontStyle style) {
		if (style == FontStyle::Arial) return L"Arial";
		if (style == FontStyle::Calibri) return L"Calibri";
	}
}