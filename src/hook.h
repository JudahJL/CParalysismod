#include "SKSE/Trampoline.h"
#include "util.h"
#include "editorID.hpp"
#pragma warning(disable: 4100)

namespace ClassicParalysis
{
	namespace Effect
	{
		inline RE::FormID SaadiaID{0x000D7505};

		void Install();
	}

	namespace Fixes
	{
		void Install();
	}

	void Install();
}