#ifndef XERCES_HPP
#define XERCES_HPP

#include <xercesc/util/PlatformUtils.hpp>

class XercesInitializer
{
public:
    XercesInitializer()
    {
        xercesc::XMLPlatformUtils::Initialize();
    }
    ~XercesInitializer()
    {
        xercesc::XMLPlatformUtils::Terminate();
    }

};

#endif // XERCES_HPP

