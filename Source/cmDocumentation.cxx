/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmDocumentation.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <utility>

#include "cmsys/FStream.hxx"
#include "cmsys/Glob.hxx"
#include "cmsys/RegularExpression.hxx"

#include "cmDocumentationEntry.h"
#include "cmDocumentationSection.h"
#include "cmRST.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmVersion.h"

namespace {
cmDocumentationEntry const cmDocumentationStandardOptions[21] = {
  { "-h,-H,--help,-help,-usage,/?", "Print usage information and exit." },
  { "--version,-version,/V [<file>]", "Print version number and exit." },
  { "--help <keyword> [<file>]", "Print help for one keyword and exit." },
  { "--help-full [<file>]", "Print all help manuals and exit." },
  { "--help-manual <man> [<file>]", "Print one help manual and exit." },
  { "--help-manual-list [<file>]", "List help manuals available and exit." },
  { "--help-command <cmd> [<file>]", "Print help for one command and exit." },
  { "--help-command-list [<file>]",
    "List commands with help available and exit." },
  { "--help-commands [<file>]", "Print cmake-commands manual and exit." },
  { "--help-module <mod> [<file>]", "Print help for one module and exit." },
  { "--help-module-list [<file>]",
    "List modules with help available and exit." },
  { "--help-modules [<file>]", "Print cmake-modules manual and exit." },
  { "--help-policy <cmp> [<file>]", "Print help for one policy and exit." },
  { "--help-policy-list [<file>]",
    "List policies with help available and exit." },
  { "--help-policies [<file>]", "Print cmake-policies manual and exit." },
  { "--help-property <prop> [<file>]",
    "Print help for one property and exit." },
  { "--help-property-list [<file>]",
    "List properties with help available and exit." },
  { "--help-properties [<file>]", "Print cmake-properties manual and exit." },
  { "--help-variable var [<file>]", "Print help for one variable and exit." },
  { "--help-variable-list [<file>]",
    "List variables with help available and exit." },
  { "--help-variables [<file>]", "Print cmake-variables manual and exit." }
};

cmDocumentationEntry const cmDocumentationCPackGeneratorsHeader = {
  {},
  "The following generators are available on this platform:"
};

cmDocumentationEntry const cmDocumentationCMakeGeneratorsHeader = {
  {},
  "The following generators are available on this platform (* marks "
  "default):"
};

bool isOption(char const* arg)
{
  return ((arg[0] == '-') || (strcmp(arg, "/V") == 0) ||
          (strcmp(arg, "/?") == 0));
}
} // anonymous namespace

cmDocumentation::cmDocumentation()
{
  this->addCommonStandardDocSections();
  this->ShowGenerators = true;
}

bool cmDocumentation::PrintVersion(std::ostream& os)
{
  /* clang-format off */
  os << this->GetNameString() << " version " << cmVersion::GetCMakeVersion();

  if (cmVersion::GetCMakeVersionExtra()[0] != '\0') {
    os << ' ' << cmVersion::GetCMakeVersionExtra();
  }

  os << "\n" <<
    "debugging support enabled" << "\n" <<
    "\n"
    "CMake suite maintained and supported by Kitware (kitware.com/cmake).\n"
    ;
  /* clang-format on */
  return true;
}

bool cmDocumentation::PrintDocumentation(Type ht, std::ostream& os)
{
  switch (ht) {
    case cmDocumentation::Usage:
      return this->PrintUsage(os);
    case cmDocumentation::Help:
      return this->PrintHelp(os);
    case cmDocumentation::Full:
      return this->PrintHelpFull(os);
    case cmDocumentation::OneArbitrary:
      return this->PrintHelpOneArbitrary(os);
    case cmDocumentation::OneManual:
      return this->PrintHelpOneManual(os);
    case cmDocumentation::OneCommand:
      return this->PrintHelpOneCommand(os);
    case cmDocumentation::OneModule:
      return this->PrintHelpOneModule(os);
    case cmDocumentation::OnePolicy:
      return this->PrintHelpOnePolicy(os);
    case cmDocumentation::OneProperty:
      return this->PrintHelpOneProperty(os);
    case cmDocumentation::OneVariable:
      return this->PrintHelpOneVariable(os);
    case cmDocumentation::ListManuals:
      return this->PrintHelpListManuals(os);
    case cmDocumentation::ListCommands:
      return this->PrintHelpListCommands(os);
    case cmDocumentation::ListModules:
      return this->PrintHelpListModules(os);
    case cmDocumentation::ListProperties:
      return this->PrintHelpListProperties(os);
    case cmDocumentation::ListVariables:
      return this->PrintHelpListVariables(os);
    case cmDocumentation::ListPolicies:
      return this->PrintHelpListPolicies(os);
    case cmDocumentation::ListGenerators:
      return this->PrintHelpListGenerators(os);
    case cmDocumentation::Version:
      return this->PrintVersion(os);
    case cmDocumentation::OldCustomModules:
      return this->PrintOldCustomModules(os);
    default:
      return false;
  }
}

bool cmDocumentation::PrintRequestedDocumentation(std::ostream& os)
{
  int count = 0;
  bool result = true;

  // Loop over requested documentation types.
  for (RequestedHelpItem const& rhi : this->RequestedHelpItems) {
    this->CurrentArgument = rhi.Argument;
    // If a file name was given, use it.  Otherwise, default to the
    // given stream.
    cmsys::ofstream fout;
    std::ostream* s = &os;
    if (!rhi.Filename.empty()) {
      fout.open(rhi.Filename.c_str());
      s = &fout;
    } else if (++count > 1) {
      os << "\n\n";
    }

    // Print this documentation type to the stream.
    if (!this->PrintDocumentation(rhi.HelpType, *s) || s->fail()) {
      result = false;
    }
  }
  return result;
}

void cmDocumentation::WarnFormFromFilename(
  cmDocumentation::RequestedHelpItem& request, bool& result)
{
  std::string ext = cmSystemTools::GetFilenameLastExtension(request.Filename);
  ext = cmSystemTools::UpperCase(ext);
  if ((ext == ".HTM") || (ext == ".HTML")) {
    request.HelpType = cmDocumentation::None;
    result = true;
    cmSystemTools::Message("Warning: HTML help format no longer supported");
  } else if (ext == ".DOCBOOK") {
    request.HelpType = cmDocumentation::None;
    result = true;
    cmSystemTools::Message("Warning: Docbook help format no longer supported");
  }
  // ".1" to ".9" should be manpages
  else if ((ext.length() == 2) && (ext[1] >= '1') && (ext[1] <= '9')) {
    request.HelpType = cmDocumentation::None;
    result = true;
    cmSystemTools::Message("Warning: Man help format no longer supported");
  }
}

std::string cmDocumentation::GeneralizeKeyword(std::string cname)
{
  std::map<std::string, std::vector<std::string> const> conversions;
  std::vector<std::string> languages = {
    "C",      "CXX",      "CSharp",      "CUDA",     "OBJC",
    "OBJCXX", "Fortran",  "HIP",         "ISPC",     "Swift",
    "ASM",    "ASM_NASM", "ASM_MARMASM", "ASM_MASM", "ASM-ATT"
  };
  std::vector<std::string> configs = { "DEBUG", "RELEASE", "RELWITHDEBINFO",
                                       "MINSIZEREL" };
  conversions.emplace("LANG", std::move(languages));
  conversions.emplace("CONFIG", std::move(configs));
  for (auto const& it : conversions) {
    for (auto const& to_replace : it.second) {
      cmsys::RegularExpression reg(
        cmStrCat("(^|_)(", to_replace, ")(\\.|$|_)"));
      if (reg.find(cname)) {
        cname.replace(reg.start(2), to_replace.length(), it.first);
      }
    }
  }
  return cname;
}

void cmDocumentation::addCommonStandardDocSections()
{
  cmDocumentationSection sec{ "Options" };
  sec.Append(cmDocumentationStandardOptions);
  this->AllSections.emplace("Options", std::move(sec));
}

void cmDocumentation::addCMakeStandardDocSections()
{
  cmDocumentationSection sec{ "Generators" };
  sec.Append(cmDocumentationCMakeGeneratorsHeader);
  this->AllSections.emplace("Generators", std::move(sec));
}

void cmDocumentation::addCTestStandardDocSections()
{
  // This is currently done for backward compatibility reason
  // We may suppress some of these.
  this->addCMakeStandardDocSections();
}

void cmDocumentation::addCPackStandardDocSections()
{
  cmDocumentationSection sec{ "Generators" };
  sec.Append(cmDocumentationCPackGeneratorsHeader);
  this->AllSections.emplace("Generators", std::move(sec));
}

bool cmDocumentation::CheckOptions(int argc, char const* const* argv,
                                   char const* exitOpt)
{
  // Providing zero arguments gives usage information.
  if (argc == 1) {
    RequestedHelpItem help;
    help.HelpType = cmDocumentation::Usage;
    this->RequestedHelpItems.push_back(std::move(help));
    return true;
  }

  auto get_opt_argument = [=](int const nextIdx, std::string& target) -> bool {
    if ((nextIdx < argc) && !isOption(argv[nextIdx])) {
      target = argv[nextIdx];
      return true;
    }
    return false;
  };

  // Search for supported help options.

  bool result = false;
  for (int i = 1; i < argc; ++i) {
    if (exitOpt && strcmp(argv[i], exitOpt) == 0) {
      return result;
    }
    RequestedHelpItem help;
    // Check if this is a supported help option.
    if ((strcmp(argv[i], "-help") == 0) || (strcmp(argv[i], "--help") == 0) ||
        (strcmp(argv[i], "/?") == 0) || (strcmp(argv[i], "-usage") == 0) ||
        (strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "-H") == 0)) {
      help.HelpType = cmDocumentation::Help;
      i += int(get_opt_argument(i + 1, help.Argument));
      // special case for arbitrary keyword help
      if (!help.Argument.empty()) {
        help.HelpType = cmDocumentation::OneArbitrary;
        i += int(get_opt_argument(i + 1, help.Filename));
      }
    } else if (strcmp(argv[i], "--help-properties") == 0) {
      help.HelpType = cmDocumentation::OneManual;
      help.Argument = "cmake-properties.7";
      i += int(get_opt_argument(i + 1, help.Filename));
      this->WarnFormFromFilename(help, result);
    } else if (strcmp(argv[i], "--help-policies") == 0) {
      help.HelpType = cmDocumentation::OneManual;
      help.Argument = "cmake-policies.7";
      i += int(get_opt_argument(i + 1, help.Filename));
      this->WarnFormFromFilename(help, result);
    } else if (strcmp(argv[i], "--help-variables") == 0) {
      help.HelpType = cmDocumentation::OneManual;
      help.Argument = "cmake-variables.7";
      i += int(get_opt_argument(i + 1, help.Filename));
      this->WarnFormFromFilename(help, result);
    } else if (strcmp(argv[i], "--help-modules") == 0) {
      help.HelpType = cmDocumentation::OneManual;
      help.Argument = "cmake-modules.7";
      i += int(get_opt_argument(i + 1, help.Filename));
      this->WarnFormFromFilename(help, result);
    } else if (strcmp(argv[i], "--help-custom-modules") == 0) {
      i += int(get_opt_argument(i + 1, help.Filename));
      cmSystemTools::Message(
        "Warning: --help-custom-modules no longer supported");
      if (help.Filename.empty()) {
        return true;
      }
      // Avoid breaking old project builds completely by at least generating
      // the output file.  Abuse help.Argument to give the file name to
      // PrintOldCustomModules without disrupting our internal API.
      help.HelpType = cmDocumentation::OldCustomModules;
      help.Argument = cmSystemTools::GetFilenameName(help.Filename);
    } else if (strcmp(argv[i], "--help-commands") == 0) {
      help.HelpType = cmDocumentation::OneManual;
      help.Argument = "cmake-commands.7";
      i += int(get_opt_argument(i + 1, help.Filename));
      this->WarnFormFromFilename(help, result);
    } else if (strcmp(argv[i], "--help-compatcommands") == 0) {
      cmSystemTools::Message(
        "Warning: --help-compatcommands no longer supported");
      return true;
    } else if (strcmp(argv[i], "--help-full") == 0) {
      help.HelpType = cmDocumentation::Full;
      i += int(get_opt_argument(i + 1, help.Filename));
      this->WarnFormFromFilename(help, result);
    } else if (strcmp(argv[i], "--help-html") == 0) {
      cmSystemTools::Message("Warning: --help-html no longer supported");
      return true;
    } else if (strcmp(argv[i], "--help-man") == 0) {
      cmSystemTools::Message("Warning: --help-man no longer supported");
      return true;
    } else if (strcmp(argv[i], "--help-command") == 0) {
      help.HelpType = cmDocumentation::OneCommand;
      i += int(get_opt_argument(i + 1, help.Argument));
      i += int(get_opt_argument(i + 1, help.Filename));
      help.Argument = cmSystemTools::LowerCase(help.Argument);
      this->WarnFormFromFilename(help, result);
    } else if (strcmp(argv[i], "--help-module") == 0) {
      help.HelpType = cmDocumentation::OneModule;
      i += int(get_opt_argument(i + 1, help.Argument));
      i += int(get_opt_argument(i + 1, help.Filename));
      this->WarnFormFromFilename(help, result);
    } else if (strcmp(argv[i], "--help-property") == 0) {
      help.HelpType = cmDocumentation::OneProperty;
      i += int(get_opt_argument(i + 1, help.Argument));
      i += int(get_opt_argument(i + 1, help.Filename));
      this->WarnFormFromFilename(help, result);
    } else if (strcmp(argv[i], "--help-policy") == 0) {
      help.HelpType = cmDocumentation::OnePolicy;
      i += int(get_opt_argument(i + 1, help.Argument));
      i += int(get_opt_argument(i + 1, help.Filename));
      this->WarnFormFromFilename(help, result);
    } else if (strcmp(argv[i], "--help-variable") == 0) {
      help.HelpType = cmDocumentation::OneVariable;
      i += int(get_opt_argument(i + 1, help.Argument));
      i += int(get_opt_argument(i + 1, help.Filename));
      this->WarnFormFromFilename(help, result);
    } else if (strcmp(argv[i], "--help-manual") == 0) {
      help.HelpType = cmDocumentation::OneManual;
      i += int(get_opt_argument(i + 1, help.Argument));
      i += int(get_opt_argument(i + 1, help.Filename));
      this->WarnFormFromFilename(help, result);
    } else if (strcmp(argv[i], "--help-command-list") == 0) {
      help.HelpType = cmDocumentation::ListCommands;
      i += int(get_opt_argument(i + 1, help.Filename));
    } else if (strcmp(argv[i], "--help-module-list") == 0) {
      help.HelpType = cmDocumentation::ListModules;
      i += int(get_opt_argument(i + 1, help.Filename));
    } else if (strcmp(argv[i], "--help-property-list") == 0) {
      help.HelpType = cmDocumentation::ListProperties;
      i += int(get_opt_argument(i + 1, help.Filename));
    } else if (strcmp(argv[i], "--help-variable-list") == 0) {
      help.HelpType = cmDocumentation::ListVariables;
      i += int(get_opt_argument(i + 1, help.Filename));
    } else if (strcmp(argv[i], "--help-policy-list") == 0) {
      help.HelpType = cmDocumentation::ListPolicies;
      i += int(get_opt_argument(i + 1, help.Filename));
    } else if (strcmp(argv[i], "--help-manual-list") == 0) {
      help.HelpType = cmDocumentation::ListManuals;
      i += int(get_opt_argument(i + 1, help.Filename));
    } else if (strcmp(argv[i], "--copyright") == 0) {
      cmSystemTools::Message("Warning: --copyright no longer supported");
      return true;
    } else if ((strcmp(argv[i], "--version") == 0) ||
               (strcmp(argv[i], "-version") == 0) ||
               (strcmp(argv[i], "/V") == 0)) {
      help.HelpType = cmDocumentation::Version;
      i += int(get_opt_argument(i + 1, help.Filename));
    }
    if (help.HelpType != None) {
      // This is a help option.  See if there is a file name given.
      result = true;
      this->RequestedHelpItems.push_back(std::move(help));
    }
  }
  return result;
}

void cmDocumentation::SetName(std::string const& name)
{
  this->NameString = name;
}

void cmDocumentation::SetSection(char const* name,
                                 cmDocumentationSection section)
{
  this->SectionAtName(name) = std::move(section);
}

cmDocumentationSection& cmDocumentation::SectionAtName(char const* name)
{
  return this->AllSections.emplace(name, cmDocumentationSection{ name })
    .first->second;
}

void cmDocumentation::AppendSection(char const* name,
                                    cmDocumentationEntry& docs)
{

  std::vector<cmDocumentationEntry> docsVec;
  docsVec.push_back(docs);
  this->AppendSection(name, docsVec);
}

void cmDocumentation::PrependSection(char const* name,
                                     cmDocumentationEntry& docs)
{

  std::vector<cmDocumentationEntry> docsVec;
  docsVec.push_back(docs);
  this->PrependSection(name, docsVec);
}

void cmDocumentation::GlobHelp(std::vector<std::string>& files,
                               std::string const& pattern)
{
  cmsys::Glob gl;
  std::string findExpr =
    cmStrCat(cmSystemTools::GetCMakeRoot(), "/Help/", pattern, ".rst");
  if (gl.FindFiles(findExpr)) {
    files = gl.GetFiles();
  }
}

void cmDocumentation::PrintNames(std::ostream& os, std::string const& pattern)
{
  std::vector<std::string> files;
  this->GlobHelp(files, pattern);
  std::vector<std::string> names;
  for (std::string const& f : files) {
    std::string line;
    cmsys::ifstream fin(f.c_str());
    while (fin && cmSystemTools::GetLineFromStream(fin, line)) {
      if (!line.empty() && (isalnum(line[0]) || line[0] == '<')) {
        names.push_back(line);
        break;
      }
    }
  }
  std::sort(names.begin(), names.end());
  for (std::string const& n : names) {
    os << n << '\n';
  }
}

bool cmDocumentation::PrintFiles(std::ostream& os, std::string const& pattern)
{
  bool found = false;
  std::vector<std::string> files;
  this->GlobHelp(files, pattern);
  std::sort(files.begin(), files.end());
  cmRST r(os, cmSystemTools::GetCMakeRoot() + "/Help");
  for (std::string const& f : files) {
    found = r.ProcessFile(f) || found;
  }
  return found;
}

bool cmDocumentation::PrintHelpFull(std::ostream& os)
{
  return this->PrintFiles(os, "index");
}

bool cmDocumentation::PrintHelpOneManual(std::ostream& os)
{
  std::string mname = this->CurrentArgument;
  std::string::size_type mlen = mname.length();
  if (mlen > 3 && mname[mlen - 3] == '(' && mname[mlen - 1] == ')') {
    mname = mname.substr(0, mlen - 3) + "." + mname[mlen - 2];
  }
  if (this->PrintFiles(os, cmStrCat("manual/", mname)) ||
      this->PrintFiles(os, cmStrCat("manual/", mname, ".[0-9]"))) {
    return true;
  }
  // Argument was not a manual.  Complain.
  os << "Argument \"" << this->CurrentArgument
     << "\" to --help-manual is not an available manual.  "
        "Use --help-manual-list to see all available manuals.\n";
  return false;
}

bool cmDocumentation::PrintHelpListManuals(std::ostream& os)
{
  this->PrintNames(os, "manual/*");
  return true;
}

bool cmDocumentation::PrintHelpOneArbitrary(std::ostream& os)
{
  std::string word = cmSystemTools::HelpFileName(this->CurrentArgument);
  std::string word_m = GeneralizeKeyword(word);

  // Support legacy style uppercase commands, with LANG and CONFIG
  // substitutions
  bool found = this->PrintFiles(os, cmStrCat("*/", word));
  if (found) {
    os << "\n";
  }
  found = this->PrintFiles(
            os, cmStrCat("command/", cmSystemTools::LowerCase(word))) ||
    found;
  if (found) {
    return true;
  }
  found = this->PrintFiles(os, cmStrCat("*/", word_m));
  if (found) {
    os << "\n";
  }
  found = this->PrintFiles(
            os, cmStrCat("command/", cmSystemTools::LowerCase(word_m))) ||
    found;
  if (found) {
    return true;
  }
  os << "Argument \"" << this->CurrentArgument
     << "\" to --help did not match any keywords.  "
        "Use --help without any arguments to print CMake help information.\n";
  return false;
}

bool cmDocumentation::PrintHelpOneCommand(std::ostream& os)
{
  std::string cname = cmSystemTools::LowerCase(this->CurrentArgument);
  if (this->PrintFiles(os, cmStrCat("command/", cname))) {
    return true;
  }
  // Argument was not a command.  Complain.
  os << "Argument \"" << this->CurrentArgument
     << "\" to --help-command is not a CMake command.  "
        "Use --help-command-list to see all commands.\n";
  return false;
}

bool cmDocumentation::PrintHelpListCommands(std::ostream& os)
{
  this->PrintNames(os, "command/*");
  return true;
}

bool cmDocumentation::PrintHelpOneModule(std::ostream& os)
{
  std::string mname = this->CurrentArgument;
  if (this->PrintFiles(os, cmStrCat("module/", mname))) {
    return true;
  }
  // Argument was not a module.  Complain.
  os << "Argument \"" << this->CurrentArgument
     << "\" to --help-module is not a CMake module.\n";
  return false;
}

bool cmDocumentation::PrintHelpListModules(std::ostream& os)
{
  std::vector<std::string> files;
  this->GlobHelp(files, "module/*");
  std::vector<std::string> modules;
  for (std::string const& f : files) {
    std::string module = cmSystemTools::GetFilenameName(f);
    modules.push_back(module.substr(0, module.size() - 4));
  }
  std::sort(modules.begin(), modules.end());
  for (std::string const& m : modules) {
    os << m << '\n';
  }
  return true;
}

bool cmDocumentation::PrintHelpOneProperty(std::ostream& os)
{
  std::string pname = cmSystemTools::HelpFileName(this->CurrentArgument);
  if (this->PrintFiles(os, cmStrCat("prop_*/", pname))) {
    return true;
  }
  // Argument was not a property.  Complain.
  os << "Argument \"" << this->CurrentArgument
     << "\" to --help-property is not a CMake property.  "
        "Use --help-property-list to see all properties.\n";
  return false;
}

bool cmDocumentation::PrintHelpListProperties(std::ostream& os)
{
  this->PrintNames(os, "prop_*/*");
  return true;
}

bool cmDocumentation::PrintHelpOnePolicy(std::ostream& os)
{
  std::string pname = this->CurrentArgument;
  std::vector<std::string> files;
  if (this->PrintFiles(os, cmStrCat("policy/", pname))) {
    return true;
  }

  // Argument was not a policy.  Complain.
  os << "Argument \"" << this->CurrentArgument
     << "\" to --help-policy is not a CMake policy.\n";
  return false;
}

bool cmDocumentation::PrintHelpListPolicies(std::ostream& os)
{
  this->PrintNames(os, "policy/*");
  return true;
}

bool cmDocumentation::PrintHelpListGenerators(std::ostream& os)
{
  auto const si = this->AllSections.find("Generators");
  if (si != this->AllSections.end()) {
    this->Formatter.PrintSection(os, si->second);
  }
  return true;
}

bool cmDocumentation::PrintHelpOneVariable(std::ostream& os)
{
  std::string vname = cmSystemTools::HelpFileName(this->CurrentArgument);
  if (this->PrintFiles(os, cmStrCat("variable/", vname))) {
    return true;
  }
  // Argument was not a variable.  Complain.
  os << "Argument \"" << this->CurrentArgument
     << "\" to --help-variable is not a defined variable.  "
        "Use --help-variable-list to see all defined variables.\n";
  return false;
}

bool cmDocumentation::PrintHelpListVariables(std::ostream& os)
{
  this->PrintNames(os, "variable/*");
  return true;
}

bool cmDocumentation::PrintUsage(std::ostream& os)
{
  auto const si = this->AllSections.find("Usage");
  if (si != this->AllSections.end()) {
    this->Formatter.PrintSection(os, si->second);
  }
  return true;
}

bool cmDocumentation::PrintHelp(std::ostream& os)
{
  auto si = this->AllSections.find("Usage");
  if (si != this->AllSections.end()) {
    this->Formatter.PrintSection(os, si->second);
  }
  si = this->AllSections.find("Options");
  if (si != this->AllSections.end()) {
    this->Formatter.PrintSection(os, si->second);
  }
  if (this->ShowGenerators) {
    si = this->AllSections.find("Generators");
    if (si != this->AllSections.end()) {
      this->Formatter.PrintSection(os, si->second);
    }
  }
  return true;
}

char const* cmDocumentation::GetNameString() const
{
  if (!this->NameString.empty()) {
    return this->NameString.c_str();
  }
  return "CMake";
}

bool cmDocumentation::PrintOldCustomModules(std::ostream& os)
{
  // CheckOptions abuses the Argument field to give us the file name.
  std::string filename = this->CurrentArgument;
  std::string ext = cmSystemTools::UpperCase(
    cmSystemTools::GetFilenameLastExtension(filename));
  std::string name = cmSystemTools::GetFilenameWithoutLastExtension(filename);

  char const* summary = "cmake --help-custom-modules no longer supported\n";
  char const* detail =
    "CMake versions prior to 3.0 exposed their internal module help page\n"
    "generation functionality through the --help-custom-modules option.\n"
    "CMake versions 3.0 and above use other means to generate their module\n"
    "help pages so this functionality is no longer available to be exposed.\n"
    "\n"
    "This file was generated as a placeholder to provide this information.\n";
  if ((ext == ".HTM") || (ext == ".HTML")) {
    os << "<html><title>" << name << "</title><body>\n"
       << summary << "<p/>\n"
       << detail << "</body></html>\n";
  } else if ((ext.length() == 2) && (ext[1] >= '1') && (ext[1] <= '9')) {
    /* clang-format off */
    os <<
      ".TH " << name << ' ' << ext[1] << " \"" <<
      cmSystemTools::GetCurrentDateTime("%B %d, %Y") <<
      "\" \"cmake " << cmVersion::GetCMakeVersion() << "\"\n"
      ".SH NAME\n"
      ".PP\n" <<
      name << " \\- " << summary <<
      "\n"
      ".SH DESCRIPTION\n"
      ".PP\n" <<
      detail
      ;
    /* clang-format on */
  } else {
    os << name << "\n\n" << summary << '\n' << detail;
  }
  return true;
}
