#ifndef ULMDOC0_ULMDOC_HPP
#define ULMDOC0_ULMDOC_HPP

#include <fstream>
#include <map>
#include <string>

namespace ulmdoc {

class UlmDoc
{
  public:
    UlmDoc() = default;

    using Key = std::string;
    using Description = std::string;

    void setActiveKey(const Key &key);
    void addMnemonicDescription(const Key &key, const Description &description);
    void addDescription(const Description &description);
    void addUnescapedDescription(const Description &description);
    void addLineBreak();
    void addFormat();
    void addPurpose();
    void addEffect();
    void addAsmNotation();
    void addAsmAlternative();

    void print(std::ifstream &tex, std::ofstream &out) const;

  private:
    Key activeKey;
    std::map<std::string, std::string> page;
    std::map<std::string, std::string> intro;
};

extern UlmDoc ulmDoc;

} // namespace ulmdoc

#endif // ULMDOC0_ULMDOC_HPP
