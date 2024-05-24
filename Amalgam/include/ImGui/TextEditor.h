#pragma once

#include <string>
#include <vector>
#include <array>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <regex>
#include "imgui.h"

class TextEditor
{
public:
	enum class PaletteIndex
	{
		Default,
		Keyword,
		Number,
		String,
		CharLiteral,
		Punctuation,
		Preprocessor,
		Identifier,
		KnownIdentifier,
		PreprocIdentifier,
		Comment,
		MultiLineComment,
		Background,
		Cursor,
		Selection,
		ErrorMarker,
		Breakpoint,
		LineNumber,
		CurrentLineFill,
		CurrentLineFillInactive,
		CurrentLineEdge,
		Max
	};

	enum class SelectionMode
	{
		Normal,
		Word,
		Line
	};

	struct Breakpoint
	{
		int mLine;
		bool mEnabled;
		std::string mCondition;

		Breakpoint()
			: mLine(-1)
			, mEnabled(false)
		{
		}
	};

	// Represents a character coordinate from the user's point of view,
	// i. e. consider an uniform grid (assuming fixed-width font) on the
	// screen as it is rendered, and each cell has its own coordinate, starting from 0.
	// Tabs are counted as [1..mTabSize] count empty spaces, depending on
	// how many space is necessary to reach the next tab stop.
	// For example, coordinate (1, 5) represents the character 'B' in a line "\tABC", when mTabSize = 4,
	// because it is rendered as "    ABC" on the screen.
	struct Coordinates
	{
		int mLine, mColumn;
		Coordinates() : mLine(0), mColumn(0) {}

		Coordinates(int aLine, int aColumn) : mLine(aLine), mColumn(aColumn)
		{
			assert(aLine >= 0);
			assert(aColumn >= 0);
		}

		static Coordinates Invalid()
		{
			static Coordinates invalid(-1, -1);
			return invalid;
		}

		bool operator ==(const Coordinates& o) const
		{
			return
				mLine == o.mLine &&
				mColumn == o.mColumn;
		}

		bool operator !=(const Coordinates& o) const
		{
			return
				mLine != o.mLine ||
				mColumn != o.mColumn;
		}

		bool operator <(const Coordinates& o) const
		{
			if (mLine != o.mLine)
			{
				return mLine < o.mLine;
			}
			return mColumn < o.mColumn;
		}

		bool operator >(const Coordinates& o) const
		{
			if (mLine != o.mLine)
			{
				return mLine > o.mLine;
			}
			return mColumn > o.mColumn;
		}

		bool operator <=(const Coordinates& o) const
		{
			if (mLine != o.mLine)
			{
				return mLine < o.mLine;
			}
			return mColumn <= o.mColumn;
		}

		bool operator >=(const Coordinates& o) const
		{
			if (mLine != o.mLine)
			{
				return mLine > o.mLine;
			}
			return mColumn >= o.mColumn;
		}
	};

	struct Identifier
	{
		Coordinates mLocation;
		std::string mDeclaration;
	};

	using String = std::string;
	using Identifiers = std::unordered_map<std::string, Identifier>;
	using Keywords = std::unordered_set<std::string>;
	using ErrorMarkers = std::map<int, std::string>;
	using Breakpoints = std::unordered_set<int>;
	using Palette = std::array<ImU32, static_cast<unsigned>(PaletteIndex::Max)>;
	using Char = uint8_t;

	struct Glyph
	{
		Char mChar;
		PaletteIndex mColorIndex = PaletteIndex::Default;
		bool mComment : 1;
		bool mMultiLineComment : 1;
		bool mPreprocessor : 1;

		Glyph(Char aChar, PaletteIndex aColorIndex) : mChar(aChar), mColorIndex(aColorIndex),
			mComment(false), mMultiLineComment(false), mPreprocessor(false)
		{
		}
	};

	using Line = std::vector<Glyph>;
	using Lines = std::vector<Line>;

	struct LanguageDefinition
	{
		using TokenRegexString = std::pair<std::string, PaletteIndex>;
		using TokenRegexStrings = std::vector<TokenRegexString>;
		using TokenizeCallback = bool(*)(const char* in_begin, const char* in_end, const char*& out_begin, const char*& out_end, PaletteIndex& paletteIndex);

		std::string mName;
		Keywords mKeywords;
		Identifiers mIdentifiers;
		Identifiers mPreprocIdentifiers;
		std::string mCommentStart, mCommentEnd, mSingleLineComment;
		char mPreprocChar;
		bool mAutoIndentation;

		TokenizeCallback mTokenize;

		TokenRegexStrings mTokenRegexStrings;

		bool mCaseSensitive;

		LanguageDefinition()
			: mPreprocChar('#'), mAutoIndentation(true), mTokenize(nullptr), mCaseSensitive(true)
		{
		}

		static const LanguageDefinition& CPlusPlus();
		static const LanguageDefinition& HLSL();
		static const LanguageDefinition& GLSL();
		static const LanguageDefinition& C();
		static const LanguageDefinition& SQL();
		static const LanguageDefinition& AngelScript();
		static const LanguageDefinition& Lua();
	};

	TextEditor();
	~TextEditor();

	void SetLanguageDefinition(const LanguageDefinition& aLanguageDef);
	const LanguageDefinition& GetLanguageDefinition() const { return mLanguageDefinition; }

	const Palette& GetPalette() const { return mPaletteBase; }
	void SetPalette(const Palette& aValue);

	void SetErrorMarkers(const ErrorMarkers& aMarkers) { mErrorMarkers = aMarkers; }
	void SetBreakpoints(const Breakpoints& aMarkers) { mBreakpoints = aMarkers; }

	void Render(const char* aTitle, const ImVec2& aSize = ImVec2(), bool aBorder = false);
	void SetText(const std::string& aText);
	std::string GetText() const;

	void SetTextLines(const std::vector<std::string>& aLines);
	std::vector<std::string> GetTextLines() const;

	std::string GetSelectedText() const;
	std::string GetCurrentLineText() const;

	int GetTotalLines() const { return static_cast<int>(mLines.size()); }
	bool IsOverwrite() const { return mOverwrite; }

	void SetReadOnly(bool aValue);
	bool IsReadOnly() const { return mReadOnly; }
	bool IsTextChanged() const { return mTextChanged; }
	bool IsCursorPositionChanged() const { return mCursorPositionChanged; }

	bool IsColorizerEnabled() const { return mColorizerEnabled; }
	void SetColorizerEnable(bool aValue);

	Coordinates GetCursorPosition() const { return GetActualCursorCoordinates(); }
	void SetCursorPosition(const Coordinates& aPosition);

	void SetHandleMouseInputs(bool aValue) { mHandleMouseInputs = aValue; }
	bool IsHandleMouseInputsEnabled() const { return mHandleKeyboardInputs; }

	void SetHandleKeyboardInputs(bool aValue) { mHandleKeyboardInputs = aValue; }
	bool IsHandleKeyboardInputsEnabled() const { return mHandleKeyboardInputs; }

	void SetImGuiChildIgnored(bool aValue) { mIgnoreImGuiChild = aValue; }
	bool IsImGuiChildIgnored() const { return mIgnoreImGuiChild; }

	void SetShowWhitespaces(bool aValue) { mShowWhitespaces = aValue; }
	bool IsShowingWhitespaces() const { return mShowWhitespaces; }

	void SetTabSize(int aValue);
	int GetTabSize() const { return mTabSize; }

	void InsertText(const std::string& aValue);
	void InsertText(const char* aValue);

	void MoveUp(int aAmount = 1, bool aSelect = false);
	void MoveDown(int aAmount = 1, bool aSelect = false);
	void MoveLeft(int aAmount = 1, bool aSelect = false, bool aWordMode = false);
	void MoveRight(int aAmount = 1, bool aSelect = false, bool aWordMode = false);
	void MoveTop(bool aSelect = false);
	void MoveBottom(bool aSelect = false);
	void MoveHome(bool aSelect = false);
	void MoveEnd(bool aSelect = false);

	void SetSelectionStart(const Coordinates& aPosition);
	void SetSelectionEnd(const Coordinates& aPosition);
	void SetSelection(const Coordinates& aStart, const Coordinates& aEnd, SelectionMode aMode = SelectionMode::Normal);
	void SelectWordUnderCursor();
	void SelectAll();
	bool HasSelection() const;

	void Copy();
	void Cut();
	void Paste();
	void Delete();

	bool CanUndo() const;
	bool CanRedo() const;
	void Undo(int aSteps = 1);
	void Redo(int aSteps = 1);

	static const Palette& GetDarkPalette();
	static const Palette& GetLightPalette();
	static const Palette& GetRetroBluePalette();

private:
	using RegexList = std::vector<std::pair<std::regex, PaletteIndex>>;

	struct EditorState
	{
		Coordinates mSelectionStart;
		Coordinates mSelectionEnd;
		Coordinates mCursorPosition;
	};

	class UndoRecord
	{
	public:
		UndoRecord() {}
		~UndoRecord() {}

		UndoRecord(
			const std::string& aAdded,
			Coordinates aAddedStart,
			Coordinates aAddedEnd,

			const std::string& aRemoved,
			Coordinates aRemovedStart,
			Coordinates aRemovedEnd,

			EditorState& aBefore,
			EditorState& aAfter);

		void Undo(TextEditor* aEditor);
		void Redo(TextEditor* aEditor);

		std::string mAdded;
		Coordinates mAddedStart;
		Coordinates mAddedEnd;

		std::string mRemoved;
		Coordinates mRemovedStart;
		Coordinates mRemovedEnd;

		EditorState mBefore;
		EditorState mAfter;
	};

	using UndoBuffer = std::vector<UndoRecord>;

	void ProcessInputs();
	void Colorize(int aFromLine = 0, int aCount = -1);
	void ColorizeRange(int aFromLine = 0, int aToLine = 0);
	void ColorizeInternal();
	float TextDistanceToLineStart(const Coordinates& aFrom) const;
	void EnsureCursorVisible();
	int GetPageSize() const;
	std::string GetText(const Coordinates& aStart, const Coordinates& aEnd) const;
	Coordinates GetActualCursorCoordinates() const;
	Coordinates SanitizeCoordinates(const Coordinates& aValue) const;
	void Advance(Coordinates& aCoordinates) const;
	void DeleteRange(const Coordinates& aStart, const Coordinates& aEnd);
	int InsertTextAt(Coordinates& aWhere, const char* aValue);
	void AddUndo(UndoRecord& aValue);
	Coordinates ScreenPosToCoordinates(const ImVec2& aPosition) const;
	Coordinates FindWordStart(const Coordinates& aFrom) const;
	Coordinates FindWordEnd(const Coordinates& aFrom) const;
	Coordinates FindNextWord(const Coordinates& aFrom) const;
	int GetCharacterIndex(const Coordinates& aCoordinates) const;
	int GetCharacterColumn(int aLine, int aIndex) const;
	int GetLineCharacterCount(int aLine) const;
	int GetLineMaxColumn(int aLine) const;
	bool IsOnWordBoundary(const Coordinates& aAt) const;
	void RemoveLine(int aStart, int aEnd);
	void RemoveLine(int aIndex);
	Line& InsertLine(int aIndex);
	void EnterCharacter(ImWchar aChar, bool aShift);
	void Backspace();
	void DeleteSelection();
	std::string GetWordUnderCursor() const;
	std::string GetWordAt(const Coordinates& aCoords) const;
	ImU32 GetGlyphColor(const Glyph& aGlyph) const;

	void HandleKeyboardInputs();
	void HandleMouseInputs();
	void Render();

	float mLineSpacing;
	Lines mLines;
	EditorState mState;
	UndoBuffer mUndoBuffer;
	int mUndoIndex;

	int mTabSize;
	bool mOverwrite;
	bool mReadOnly;
	bool mWithinRender;
	bool mScrollToCursor;
	bool mScrollToTop;
	bool mTextChanged;
	bool mColorizerEnabled;
	float mTextStart; // position (in pixels) where a code line starts relative to the left of the TextEditor.
	int mLeftMargin;
	bool mCursorPositionChanged;
	int mColorRangeMin, mColorRangeMax;
	SelectionMode mSelectionMode;
	bool mHandleKeyboardInputs;
	bool mHandleMouseInputs;
	bool mIgnoreImGuiChild;
	bool mShowWhitespaces;

	Palette mPaletteBase{};
	Palette mPalette{};
	LanguageDefinition mLanguageDefinition;
	RegexList mRegexList;

	bool mCheckComments;
	Breakpoints mBreakpoints;
	ErrorMarkers mErrorMarkers;
	ImVec2 mCharAdvance;
	Coordinates mInteractiveStart, mInteractiveEnd;
	std::string mLineBuffer;
	uint64_t mStartTime;

	float mLastClick;
};
