#include "OGCore/OGDependencies.h"
#include <Window/Window.h>
#include "Graphics/GraphicsInterface.h"

using namespace oi::wc;
using namespace oi::gc;
using namespace oi;

int main() {
	Window &w = Window::create(WindowInfo("Graphics test"));
	w.setInterface(new GraphicInterface());
	w.waitAll();

	return 0;
}