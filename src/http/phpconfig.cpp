/*****************************************************************************
*    Open LiteSpeed is an open source HTTP server.                           *
*    Copyright (C) 2013 - 2018  LiteSpeed Technologies, Inc.                 *
*                                                                            *
*    This program is free software: you can redistribute it and/or modify    *
*    it under the terms of the GNU General Public License as published by    *
*    the Free Software Foundation, either version 3 of the License, or       *
*    (at your option) any later version.                                     *
*                                                                            *
*    This program is distributed in the hope that it will be useful,         *
*    but WITHOUT ANY WARRANTY; without even the implied warranty of          *
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the            *
*    GNU General Public License for more details.                            *
*                                                                            *
*    You should have received a copy of the GNU General Public License       *
*    along with this program. If not, see http://www.gnu.org/licenses/.      *
*****************************************************************************/
#include "phpconfig.h"
#include <extensions/lsapi/lsapireq.h>
#include <util/stringtool.h>

#include <ctype.h>
#include <string.h>




int PHPValue::setValue(const char *pKey, const char *pValue,
                       short iType)
{
    if ((!pKey) || (!pValue))
        return LS_FAIL;
    if (iType != PHP_CONF_SYSTEM)
        iType = PHP_CONF_PERDIR;
    int keyLen = strlen(pKey);
    if (!m_sKey.prealloc(keyLen + 3))
        return LS_FAIL;
    char *keyBuf = m_sKey.buf();
    *keyBuf++ = PHP_CONFIG_ENV;
    *keyBuf++ = iType;
    memmove(keyBuf, pKey, keyLen + 1);
    m_sKey.setLen(keyLen + 2);
    m_sVal = pValue;
    return 0;
}

PHPConfig::PHPConfig()
    : m_lsapiEnv(256)
{
}


PHPConfig::PHPConfig(const PHPConfig &rhs)
    : m_lsapiEnv(256)
{
    HashStringMap< PHPValue *>::iterator iter;
    for (iter = rhs.begin();
         iter != rhs.end();
         iter = rhs.next(iter))
    {
        PHPValue *pVal = new PHPValue(*(iter.second()));
        if (pVal)
            insert(pVal->getConfigKey(), pVal);
    }
    buildLsapiEnv();
}


PHPConfig::~PHPConfig()
{
    release_objects();
}




int PHPConfig::merge(const PHPConfig *pParent)
{
    if (!pParent)
        return 0;
    HashStringMap< PHPValue *>::iterator iter, iter1;
    for (iter = pParent->begin();
         iter != pParent->end();
         iter = pParent->next(iter))
    {
        iter1 = find(iter.first());
        if (iter1 != end())
        {
            short parentType = iter.second()->getType();
            if ((parentType == PHP_CONF_SYSTEM) &&
                (iter1.second()->getType() != PHP_CONF_SYSTEM))
            {
                iter1.second()->setType(iter.second()->getType());
                iter1.second()->setValue(iter.second()->getValue());
            }
        }
        else
        {
            PHPValue *pVal = new PHPValue(*(iter.second()));
            if (pVal)
                insert(pVal->getConfigKey(), pVal);
        }
    }
    return 1;
}

int PHPConfig::parse(int id, const char *pArgs,
                     char *pErr, int errBufLen)
{
    int iType = 0;
    char achBuf[4096];
    if (!pArgs)
        return LS_FAIL;

    memccpy(achBuf, pArgs, 0, 4095);
    achBuf[4095] = 0;
    char *pArg1 = achBuf;
    while (isspace(*pArg1))
        ++pArg1;

    const char *pArg2 = StringTool::strNextArg(pArg1);
    if (pArg2)
        *(char *)pArg2++ = 0;
    else
        return LS_FAIL;
    StringTool::strlower(pArg1, pArg1);
    const char *pArg2End = pArg2 + strlen(pArg2);
    StringTool::strTrim(pArg2, pArg2End);
    if (*pArg2 == '"')
    {
        ++pArg2;
        if (*(pArg2End - 1) == '"')
            --pArg2End;
    }
    StringTool::strTrim(pArg2, pArg2End);
    *(char *)pArg2End = 0;
    if ((id == PHP_FLAG) ||
        (id == PHP_ADMIN_FLAG))
    {
        if ((strcasecmp(pArg2, "On") == 0) ||
            (strcmp(pArg2, "1") == 0))
            strcpy((char *)pArg2, "1");
        else
            strcpy((char *)pArg2, "0");
    }
    if ((id == PHP_ADMIN_VALUE) ||
        (id == PHP_ADMIN_FLAG))
        iType = PHP_CONF_SYSTEM;
    else
        iType = PHP_CONF_PERDIR;

    iterator iter = find(pArg1);
    if (iter != end())
    {
        iter.second()->setType(iType);
        iter.second()->setValue(pArg2);
    }
    else
    {
        PHPValue *pVal = new PHPValue();
        if (!pVal)
        {
            memccpy(pErr, "Out of memory", 0, errBufLen);
            return 1;
        }
        pVal->setValue(pArg1, pArg2, iType);
        insert(pVal->getConfigKey(), pVal);
    }
    return 0;
}

int PHPConfig::buildLsapiEnv()
{
    iterator iter;
    m_lsapiEnv.clear();
    for (iter = begin();
         iter != end();
         iter = next(iter))
    {
        PHPValue *pVal = iter.second();
        if (LsapiReq::addEnv(&m_lsapiEnv, pVal->getKey(), pVal->getKeyLen(),
                             pVal->getValue(), pVal->getValLen()) == -1)
            return LS_FAIL;
    }
    return 0;
}
