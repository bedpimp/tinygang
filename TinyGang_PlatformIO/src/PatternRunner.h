#ifndef __PATTERNRUNNER__H__
#define __PATTERNRUNNER__H__

#include <elapsedMillis.h>

#include "patterns/BassShader.h"
#include "patterns/BodyTwinkler.h"
#include "patterns/BookendFlip.h"
#include "patterns/BookendTrace.h"
#include "patterns/RainbowSparkle.h"
#include "patterns/Twinkler.h"
#include "patterns/WhiteTrace.h"

Pattern *patterns[] = {
	new BodyTwinkler(),    // 0 red yellow sparkle short
	new BassShader(),      // 1 pattern individually triggered
	new RainbowSparkle(),  // 2 more pale pink and blue
	new WhiteTrace(),      // 3
	new BookendTrace(),    // 4
	new Twinkler(),        // 5
	new BookendFlip()      // 6
};

int PATTERN_HUE[] = {0, 20, 255, 229, 120, 200, 207};
// 120 should be green, not cyan
// 229 pink
// 22 orange
// 200 lilac
// 'a' green
const size_t PATTERNS_COUNT = (sizeof(patterns)) / 4;

static_assert(
	PATTERNS_COUNT == (sizeof(PATTERN_HUE) / sizeof(PATTERN_HUE[0])),
	"PATTERN_HUE doesn't match size of PATTERNS_COUNT");



char patternCommand[] = {
	'q', 'a', 'z',
	'w', 's', 'x',
	'e', 'd', 'c',
	'r', 'f', 'v'};
const size_t SERIALIZED_PATTERN_COUNT = std::min(PATTERNS_COUNT, sizeof(patternCommand));

struct SerializedPattern {
	char commandChar;
    
    SerializedPattern(uint32_t patternIndex = 0) {
        if(patternIndex < SERIALIZED_PATTERN_COUNT) {
            //Lookup command in array
            commandChar = patternCommand[patternIndex];
        } else {
            //Normally would throw, but since exceptions are disabled...
            //On error just return first
            commandChar = patternCommand[0];
        }
        
    }
    
    String toString() {
        return String(commandChar);
    }
};

int DeserializePattern(SerializedPattern serializedPattern) {
	//Search by index into patternCommand
	for (int i = 0; i < std::min(PATTERNS_COUNT, SERIALIZED_PATTERN_COUNT); i++) {
		if (serializedPattern.commandChar == patternCommand[i]) {
			return i;
		}
	}
	return -1;
}

template <unsigned int LED_COUNT>
class PatternRunner {
   private:
	elapsedMillis m_ellapseTimeMs[PATTERNS_COUNT];
	// CRGB led[PATTERNS_COUNT][LED_COUNT];

	unsigned long m_durationMs;

   public:
	// Just making this buffer public to avoid more complicated getters
	CRGB m_outBuffer[LED_COUNT];

	PatternRunner(unsigned long duration = 1000) : m_durationMs(duration) {}
	~PatternRunner() = default;

	// Getters
	unsigned long PatternElapsed(int patternId) const {
		return m_ellapseTimeMs[patternId];
	}

	bool PatternActive(int patternId) const {
		return m_ellapseTimeMs[patternId] < m_durationMs;
	}

	size_t PatternCount() const { return PATTERNS_COUNT; }

	// Setter
	void StartPattern(int patternId) {
		Serial.printf("%u: PatternRunner: Starting pattern %i\n", millis(), patternId);
		m_ellapseTimeMs[patternId] = 0;
	}

	// Key Callbacks

	// Called on main.setup
	void setup() {
		for (int i = 0; i < LED_COUNT; i++) {
			m_outBuffer[i] = CHSV(255 / LED_COUNT * i, 255 / LED_COUNT * i, 255);
		}
	}

   private:
	uint32_t m_lastPatternMask = 0;

	uint32_t activePatternMask() {
		uint32_t patternMask = 0;
		for (int i = 0; i < (PATTERNS_COUNT > 32 ? 32 : PATTERNS_COUNT); i++) {
			if (!PatternActive(i)) continue;
			patternMask |= (1 << i);
		}
		return patternMask;
	}

   public:
	// TODO: migrate to .cpp?
	void updateLedColors() {
		int activeCt = 0;
		for (int i = 0; i < PATTERNS_COUNT; i++) {
			if (!PatternActive(i)) continue;
			activeCt++;
			// if(activeCt > 1) continue;
			for (int j = 0; j < LED_COUNT; j++) {
				float position = (float)j / (float)LED_COUNT;
				float remaining = 1.0 - (float)m_ellapseTimeMs[i] / (float)m_durationMs;
				// TODO: re-introduce the concept of inboundHue (renamed to legacy_inbound_hue), which isn't sent / changed currently
				//  This also includes the idea of self color => pattern_hue lookup vs inbound hue for self vs other patterns
				m_outBuffer[j] = patterns[i]->paintLed(position, remaining, m_outBuffer[j], PATTERN_HUE[i]);
			}
		}

		// if(PatternActive(3)) {
		//     //debug!
		//     Serial.printf("%u: DEBUG PATTERN3!. BITDUMP:\n BITS:[", millis());
		//     for (int j = 0; j < LED_COUNT; j++) {
		//         Serial.printf("%02x, ", m_outBuffer[j].getLuma());
		//     }
		//     Serial.println("]\n");
		// }

		uint32_t activeMask = activePatternMask();

		if (activeMask != m_lastPatternMask) {
			Serial.printf("%u: PatternRunner: %i Active Patterns. PatternMask:%i\n", millis(), activeCt, activeMask);
			if (activeCt == 0) Serial.print("    *All Patterns Dormant*\n");
		}

		if (activeCt == 0) {
			// Slowly fade out
			for (int j = 0; j < LED_COUNT; j++) {
				m_outBuffer[j] = m_outBuffer[j].nscale8(190);
			}
		}

		m_lastPatternMask = activeMask;
	}
};

#endif  //!__PATTERNRUNNER__H__