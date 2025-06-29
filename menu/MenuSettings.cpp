#include "MenuSettings.h"

namespace settings {
    bool enabled = true;
	bool injected = false;
	ImVec2 windowSize = ImVec2(400, 300);
	ImVec2 codePanelSize = ImVec2(400, 300);

	void setInjected(bool injected) {
		settings::injected = injected;
	}
}