#include <format>
#include <iostream>
#include <string>
int main(){
#ifdef MFP_BAD_LITERAL
    auto bad=std::format("{:d}","x");return bad.empty();
#else
    if(std::format("{}:{:x}",42,255)!="42:ff")return 1;
    std::string word="x";bool refused=false;
    try{(void)std::vformat("{:d}",std::make_format_args(word));}
    catch(const std::format_error&){refused=true;}
    if(!refused)return 1;
    std::cout<<"MFP_CPP checked-literal runtime-format-refusal\n";
#endif
}
