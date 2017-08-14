#pragma once

namespace patterns
{
	namespace detail
	{
		constexpr uint8_t hex(char c)
		{
			return (c >= '0' && c <= '9') ? static_cast<uint8_t>(c - '0')
				: (c >= 'a' && c <= 'f') ? static_cast<uint8_t>(c - 'a' + 10)
					: (c >= 'A' && c <= 'F') ? static_cast<uint8_t>(c - 'A' + 10)
						: throw std::domain_error("not a hex digit");
		}

		constexpr bool ishex(char c)
		{
			return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
		}
	}

	constexpr size_t count_bytes(const char* pattern)
	{
		size_t count = 0;

		for (; pattern[0]; ++pattern) {
			if (pattern[0] == ' ')
				continue;

			if (detail::ishex(pattern[0])) {
				if (!detail::ishex((++pattern)[0]))
					throw std::logic_error("the second hex digit is missing");

				++count;
				continue;
			}

			if (pattern[0] == '?') {
				if ((++pattern)[0] != '?')
					throw std::logic_error("the second question mark is missing");

				++count;
				continue;
			} else {
				throw std::domain_error("only hex digits, spaces and question marks are allowed");
			}
		}

		return count;
	}

	template<size_t PatternLength>
	struct Pattern
	{
		uint8_t bytes[PatternLength];
		char mask[PatternLength];

		constexpr Pattern(const char* pattern)
			: bytes()
			, mask()
		{
			// Note that some input validation is absent from here,
			// because the input is expected to have already been validated in count_bytes().
			size_t i = 0;

			for (; pattern[0]; ++pattern) {
				if (pattern[0] == ' ')
					continue;

				if (detail::ishex(pattern[0])) {
					bytes[i] = detail::hex(pattern[0]) * 16 + detail::hex(pattern[1]);
					mask[i++] = 'x';

					++pattern;
					continue;
				}

				if (pattern[0] == '?') {
					mask[i++] = '?';

					++pattern;
					continue;
				}

				throw std::domain_error("only hex digits, spaces and question marks are allowed");
			}

			if (i != PatternLength)
				throw std::logic_error("wrong pattern length");
		}
	};

	#define PATTERN(pattern) \
		::patterns::Pattern<::patterns::count_bytes(pattern)>(pattern)

	class PatternWrapper
	{
		const char* name_;
		const uint8_t* bytes;
		const char* mask;
		const size_t length_;

	public:
		template<size_t PatternLength>
		constexpr PatternWrapper(const char* name, const Pattern<PatternLength>& pattern)
			: name_(name)
			, bytes(pattern.bytes)
			, mask(pattern.mask)
			, length_(PatternLength)
		{
		}

		template<size_t PatternLength>
		constexpr PatternWrapper(const Pattern<PatternLength>& pattern)
			: PatternWrapper("", pattern)
		{
		}

		constexpr const char* name() const
		{
			return name_;
		}

		constexpr size_t length() const
		{
			return length_;
		}

		inline bool match(const uint8_t* memory) const
		{
			for (size_t i = 0; i < length_; ++i)
				if (mask[i] == 'x' && memory[i] != bytes[i])
					return false;

			return true;
		}
	};

	template<class... Pattern>
	constexpr std::array<PatternWrapper, sizeof...(Pattern)> make_pattern_array(const Pattern&... patterns)
	{
		return{ PatternWrapper(patterns)... };
	}

	#define CONCATENATE1(arg1, arg2) arg1 ## arg2
	#define CONCATENATE(arg1, arg2) CONCATENATE1(arg1, arg2)

	/*
	 * Concatenate with empty because otherwise the MSVC preprocessor
	 * puts all __VA_ARGS__ arguments into the first one.
	 */
	#define MAKE_PATTERN_1(name, pattern_name, pattern, ...) \
		static constexpr auto ptn_ ## name ## _1 = PATTERN(pattern);
	#define MAKE_PATTERN_2(name, pattern_name, pattern, ...) \
		static constexpr auto ptn_ ## name ## _2 = PATTERN(pattern); \
		CONCATENATE(MAKE_PATTERN_1(name, __VA_ARGS__),)
	#define MAKE_PATTERN_3(name, pattern_name, pattern, ...) \
		static constexpr auto ptn_ ## name ## _3 = PATTERN(pattern); \
		CONCATENATE(MAKE_PATTERN_2(name, __VA_ARGS__),)
	#define MAKE_PATTERN_4(name, pattern_name, pattern, ...) \
		static constexpr auto ptn_ ## name ## _4 = PATTERN(pattern); \
		CONCATENATE(MAKE_PATTERN_3(name, __VA_ARGS__),)
	#define MAKE_PATTERN_5(name, pattern_name, pattern, ...) \
		static constexpr auto ptn_ ## name ## _5 = PATTERN(pattern); \
		CONCATENATE(MAKE_PATTERN_4(name, __VA_ARGS__),)
	#define MAKE_PATTERN_6(name, pattern_name, pattern, ...) \
		static constexpr auto ptn_ ## name ## _6 = PATTERN(pattern); \
		CONCATENATE(MAKE_PATTERN_5(name, __VA_ARGS__),)
	#define MAKE_PATTERN_7(name, pattern_name, pattern, ...) \
		static constexpr auto ptn_ ## name ## _7 = PATTERN(pattern); \
		CONCATENATE(MAKE_PATTERN_6(name, __VA_ARGS__),)
	#define MAKE_PATTERN_8(name, pattern_name, pattern, ...) \
		static constexpr auto ptn_ ## name ## _8 = PATTERN(pattern); \
		CONCATENATE(MAKE_PATTERN_7(name, __VA_ARGS__),)

	/*
	 * Cannot concatenate with empty in a way compatible with non-MSVC
	 * here because there are commas inside of the macros.
	 */
#ifdef _MSC_VER // MSVC
	#define NAME_PATTERN_1(name, pattern_name, pattern, ...) \
		PatternWrapper{ pattern_name, ptn_ ## name ## _1 }
	#define NAME_PATTERN_2(name, pattern_name, pattern, ...) \
		PatternWrapper{ pattern_name, ptn_ ## name ## _2 }, \
		CONCATENATE(NAME_PATTERN_1(name, __VA_ARGS__),)
	#define NAME_PATTERN_3(name, pattern_name, pattern, ...) \
		PatternWrapper{ pattern_name, ptn_ ## name ## _3 }, \
		CONCATENATE(NAME_PATTERN_2(name, __VA_ARGS__),)
	#define NAME_PATTERN_4(name, pattern_name, pattern, ...) \
		PatternWrapper{ pattern_name, ptn_ ## name ## _4 }, \
		CONCATENATE(NAME_PATTERN_3(name, __VA_ARGS__),)
	#define NAME_PATTERN_5(name, pattern_name, pattern, ...) \
		PatternWrapper{ pattern_name, ptn_ ## name ## _5 }, \
		CONCATENATE(NAME_PATTERN_4(name, __VA_ARGS__),)
	#define NAME_PATTERN_6(name, pattern_name, pattern, ...) \
		PatternWrapper{ pattern_name, ptn_ ## name ## _6 }, \
		CONCATENATE(NAME_PATTERN_5(name, __VA_ARGS__),)
	#define NAME_PATTERN_7(name, pattern_name, pattern, ...) \
		PatternWrapper{ pattern_name, ptn_ ## name ## _7 }, \
		CONCATENATE(NAME_PATTERN_6(name, __VA_ARGS__),)
	#define NAME_PATTERN_8(name, pattern_name, pattern, ...) \
		PatternWrapper{ pattern_name, ptn_ ## name ## _8 }, \
		CONCATENATE(NAME_PATTERN_7(name, __VA_ARGS__),)
#else // Not MSVC
	#define NAME_PATTERN_1(name, pattern_name, pattern, ...) \
		PatternWrapper{ pattern_name, ptn_ ## name ## _1 }
	#define NAME_PATTERN_2(name, pattern_name, pattern, ...) \
		PatternWrapper{ pattern_name, ptn_ ## name ## _2 }, \
		NAME_PATTERN_1(name, __VA_ARGS__)
	#define NAME_PATTERN_3(name, pattern_name, pattern, ...) \
		PatternWrapper{ pattern_name, ptn_ ## name ## _3 }, \
		NAME_PATTERN_2(name, __VA_ARGS__)
	#define NAME_PATTERN_4(name, pattern_name, pattern, ...) \
		PatternWrapper{ pattern_name, ptn_ ## name ## _4 }, \
		NAME_PATTERN_3(name, __VA_ARGS__)
	#define NAME_PATTERN_5(name, pattern_name, pattern, ...) \
		PatternWrapper{ pattern_name, ptn_ ## name ## _5 }, \
		NAME_PATTERN_4(name, __VA_ARGS__)
	#define NAME_PATTERN_6(name, pattern_name, pattern, ...) \
		PatternWrapper{ pattern_name, ptn_ ## name ## _6 }, \
		NAME_PATTERN_5(name, __VA_ARGS__)
	#define NAME_PATTERN_7(name, pattern_name, pattern, ...) \
		PatternWrapper{ pattern_name, ptn_ ## name ## _7 }, \
		NAME_PATTERN_6(name, __VA_ARGS__)
	#define NAME_PATTERN_8(name, pattern_name, pattern, ...) \
		PatternWrapper{ pattern_name, ptn_ ## name ## _8 }, \
		NAME_PATTERN_7(name, __VA_ARGS__)
#endif

	#define FOR_EACH2_RSEQ_N 8, 0, 7, 0, 6, 0, 5, 0, 4, 0, 3, 0, 2, 0, 1
	#define FOR_EACH2_ARG_N(__1, __1_, __2, __2_, __3, __3_, __4, __4_, __5, __5_, __6, __6_, __7, __7_, __8, __8_, N, ...) N
	#define FOR_EACH2_NARG_(...) CONCATENATE(FOR_EACH2_ARG_N(__VA_ARGS__),)
	#define FOR_EACH2_NARG(...) FOR_EACH2_NARG_(__VA_ARGS__, FOR_EACH2_RSEQ_N)

	#define MAKE_PATTERNS_(N, ...) CONCATENATE(CONCATENATE(MAKE_PATTERN_, N)(__VA_ARGS__),)
	#define MAKE_PATTERNS(name, ...) MAKE_PATTERNS_(FOR_EACH2_NARG(__VA_ARGS__), name, __VA_ARGS__)

#ifdef _MSC_VER // MSVC
	#define NAME_PATTERNS_(N, ...) CONCATENATE(CONCATENATE(NAME_PATTERN_, N)(__VA_ARGS__),)
	#define NAME_PATTERNS(name, ...) NAME_PATTERNS_(FOR_EACH2_NARG(__VA_ARGS__), name, __VA_ARGS__)
#else // Not MSVC
	#define NAME_PATTERNS_(N, ...) CONCATENATE(NAME_PATTERN_, N)(__VA_ARGS__)
	#define NAME_PATTERNS(name, ...) NAME_PATTERNS_(FOR_EACH2_NARG(__VA_ARGS__), name, __VA_ARGS__)
#endif

	/*
	 * Defines an array of compile-time patterns.
	 * Example:
	 *
	 * 	PATTERNS(MyPattern,
	 *		"HL-SteamPipe",
	 *		"11 22 33 ?? ?? FF AC",
	 *		"HL-NGHL",
	 *		"C0 AB 22 33 ?? 11"
	 *	);
	 *
	 * is converted into:
	 *
	 *	static constexpr auto ptn_MyPattern_2 = ::patterns::Pattern<::patterns::count_bytes("11 22 33 ?? ?? FF AC")>("11 22 33 ?? ?? FF AC");
	 *	static constexpr auto ptn_MyPattern_1 = ::patterns::Pattern<::patterns::count_bytes("C0 AB 22 33 ?? 11")>("C0 AB 22 33 ?? 11");
	 *	constexpr auto MyPattern = ::patterns::make_pattern_array(
	 *		PatternWrapper{ "HL-SteamPipe", ptn_MyPattern_2 },
	 *		PatternWrapper{ "HL-NGHL", ptn_MyPattern_1 }
	 *	);
	 */
	#define PATTERNS(name, ...) \
		MAKE_PATTERNS(name, __VA_ARGS__) \
		constexpr auto name = ::patterns::make_pattern_array( \
			NAME_PATTERNS(name, __VA_ARGS__) \
		);
}
