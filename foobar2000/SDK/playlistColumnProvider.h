#pragma once

namespace fb2k {
	//! Declares a column to be made available in Default UI playlist view, \n
	//! without user having to manually enter title formatting patterns.
	class playlistColumnProvider : public service_base {
		FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(playlistColumnProvider);
	public:
		//! Number of columns published by this object.
		virtual size_t numColumns() = 0;
		//! Unique identifier of a column.
		virtual GUID columnID(size_t col) = 0;
		//! Formatting pattern used for displaying a column.
		virtual fb2k::stringRef columnFormatSpec(size_t col) = 0;
		//! Optional; sorting pattern used for this column. Return null to use display pattern for sorting.
		virtual fb2k::stringRef columnSortScript(size_t col) = 0;
		//! Name of the column shown to the user.
		virtual fb2k::stringRef columnName(size_t col) = 0;
		//! Display flags (alignment). \n
		//! See flag_* constants.
		virtual unsigned columnFlags(size_t col) = 0;

		static constexpr unsigned
            flag_alignLeft = 0, flag_alignRight = 1 << 0, flag_alignCenter = 1 << 1, // alignment
            flag_numeric = 1 << 2, // prefer fixed width font, not all renderers support this
            flag_positionDependant = 1 << 3, // value changes with position in playlist, mainly used by list index etc
            flag_glyphs = 1 << 4; // internal/reserved
		static constexpr unsigned flag_alignMask = (flag_alignLeft|flag_alignRight|flag_alignCenter);
	};
}
