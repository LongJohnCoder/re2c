#ifndef _RE2C_GLOBALS_
#define _RE2C_GLOBALS_

#include <string>

#include "src/codegen/code_names.h"
#include "src/codegen/input_api.h"
#include "src/conf/warn.h"
#include "src/ir/regexp/encoding/enc.h"
#include "src/ir/regexp/empty_class_policy.h"
#include "src/util/c99_stdint.h"

namespace re2c
{

extern bool bFlag;
extern bool cFlag;
extern bool dFlag;
extern bool DFlag;
extern bool fFlag;
extern bool FFlag;
extern bool gFlag;
extern bool iFlag;
extern bool rFlag;
extern bool sFlag;
extern bool tFlag;
extern bool flag_skeleton;

extern bool bNoGenerationDate;
extern bool bUsedYYBitmap;

extern std::string labelPrefix;
extern std::string condPrefix;
extern std::string condEnumPrefix;
extern std::string condDivider;
extern std::string condDividerParam;
extern std::string condGoto;
extern std::string condGotoParam;
extern std::string yychConversion;
extern std::string yyFillLength;
extern std::string yySetConditionParam;
extern std::string yySetStateParam;
extern std::string yySetupRule;
extern uint32_t cGotoThreshold;

/* configurations */
extern uint32_t topIndent;
extern std::string indString;
extern bool yybmHexTable;
extern bool bEmitYYCh;
extern bool bUseStateAbort;
extern bool bUseStateNext;
extern bool bUseYYFill;
extern bool bUseYYFillParam;
extern bool bUseYYFillCheck;
extern bool bUseYYFillNaked;
extern bool bUseYYSetConditionParam;
extern bool bUseYYGetConditionNaked;
extern bool bUseYYSetStateParam;
extern bool bUseYYSetStateNaked;
extern bool bUseYYGetStateNaked;
extern bool bWroteGetState;
extern bool bWroteCondCheck;
extern bool bCaseInsensitive;
extern bool bCaseInverted;

extern const uint32_t asc2asc[256];
extern const uint32_t asc2ebc[256];
extern const uint32_t ebc2asc[256];

extern uint32_t last_fill_index;
extern CodeNames mapCodeName;

extern Enc encoding;
extern InputAPI input_api;

extern empty_class_policy_t empty_class_policy;

extern Warn warn;

} // end namespace re2c

#endif // _RE2C_GLOBALS_
