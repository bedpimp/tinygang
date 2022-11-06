#ifndef __PATTERNSERIALIZATION__H__
#define __PATTERNSERIALIZATION__H__

/* *********************
Pattern serialization is getting complicated because of what it means to be a "pattern"
Need to rename some concepts...

The "Pattern" class might be renamed "PatternFunction"
    It defines a single function to compute the current value for a pixel.

These patterns are stored in a fixed library / array, referenced through
an integer index. This index has now been typedef'd to a PatternReference.

A PatternReference is serialized and deserialized for network communication by
conversion into a single char by using the PATTERN_COMMANDS array.

********************* */

// For String
#include "Arduino.h"
#include "patterns/BassShader.h"
#include "patterns/BodyTwinkler.h"
#include "patterns/BookendFlip.h"
#include "patterns/BookendTrace.h"
#include "patterns/RainbowSparkle.h"
#include "patterns/Twinkler.h"
#include "patterns/WhiteTrace.h"

// Number of patterns. Must be compile time constant
constexpr size_t PATTERNS_COUNT = 7;
extern Pattern *patterns[PATTERNS_COUNT];

constexpr char PATTERN_COMMANDS[] = {
	'q', 'a', 'z',
	'w', 's', 'x',
	'e', 'd', 'c',
	'r', 'f', 'v'};
const size_t SERIALIZED_PATTERN_COUNT = std::min(PATTERNS_COUNT, sizeof(PATTERN_COMMANDS));

//Static hue, to be replaced by network comms
constexpr int PATTERN_HUE[] = {0, 20, 255, 229, 120, 200, 207};
// 120 should be green, not cyan
// 229 pink
// 22 orange
// 200 lilac
// 'a' green
static_assert(
	PATTERNS_COUNT == (sizeof(PATTERN_HUE) / sizeof(PATTERN_HUE[0])),
	"PATTERN_HUE doesn't match size of PATTERNS_COUNT");

// Intentionally using signed int so we can use negative as invalid flag;
using PatternReference = int;
const PatternReference INVALID_PATTERN_REF = -1;
inline bool PatternRefValid(PatternReference ref) {
	return ref >= 0 && ref < SERIALIZED_PATTERN_COUNT;
}

// Each user (gang member) on the mesh gets to pick and broadcast
// their own custom pattern. This custom pattern is a pattern algorithm (patternRef)
// plus a pattern color (hue)
// This data will be broadcast and synchronized across the mesh
struct SharedNodeData {
	PatternReference nodePattern;
	uint8_t hue;
	// todo: add hue
	
	
	SharedNodeData(PatternReference pattern = 0) {
		nodePattern = pattern;
		resetDefaultHue();
	}

	bool isValid() {
		return PatternRefValid(nodePattern);
	}
	
	void resetDefaultHue() {
		if(isValid()){
			hue = PATTERN_HUE[nodePattern];	
		} else {
			hue = 0;
		}
	}
};

struct SerializedNodeData {
	char commandChar;

	SerializedNodeData(char command) {
		commandChar = command;
		// TODO: verify valid?
	}

	SerializedNodeData(PatternReference patternIndex = 0) {
		if (PatternRefValid(patternIndex)) {
			// Lookup command in array
			commandChar = PATTERN_COMMANDS[patternIndex];
		} else {
			// Normally would throw, but since exceptions are disabled...
			// On error just return first
			commandChar = PATTERN_COMMANDS[0];
		}
	}

	String toString() {
		return String(commandChar);
	}
};

inline SerializedNodeData SerializeNodeData(SharedNodeData nodeData) {
	return SerializedNodeData(nodeData.nodePattern);
}

inline PatternReference DeserializePatternRef(char commandChar) {
	SharedNodeData nodeData;

	// Search by index into PATTERN_COMMANDS
	for (int i = 0; i < std::min(PATTERNS_COUNT, SERIALIZED_PATTERN_COUNT); i++) {
		if (commandChar == PATTERN_COMMANDS[i]) {
			return i;
		}
	}

	// Invalid
	return INVALID_PATTERN_REF;
}

inline SharedNodeData DeserializeNodeData(SerializedNodeData serializedData) {
	SharedNodeData nodeData;
	nodeData.nodePattern = DeserializePatternRef(serializedData.commandChar);
	//TODO: read hue
	nodeData.resetDefaultHue();
	return nodeData;
}

#endif  //!__PATTERNSERIALIZATION__H__