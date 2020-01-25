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
#include "modulemanager.h"

#include <http/handlertype.h>
#include <http/httplog.h>
#include <log4cxx/logger.h>
#include <lsiapi/internal.h>
#include <lsiapi/lsiapi.h>
#include <lsiapi/lsiapihooks.h>
#include <lsiapi/lsiapilib.h>
#include <lsiapi/moduletimer.h>
#include <main/mainserverconfig.h>
#include <http/httpserverconfig.h>
#include <util/xmlnode.h>
#include <ls.h>
#include <lsr/ls_confparser.h>
#include <dlfcn.h>

#include <util/stringtool.h>
#define INITIAL_MODULE_COUNT   10


LsiModule::LsiModule(lsi_module_t *pModule)
    : HttpHandler(HandlerType::HT_MODULE)
{
    m_pModule = pModule;
}


LS_SINGLETON(ModuleManager);


ModuleManager::ModuleManager()
{
    m_pModuleArray = NULL;
    m_pGlobalModuleConfig = new ModuleConfig();
}


ModuleManager::~ModuleManager()
{
    if (m_pGlobalModuleConfig)
        delete m_pGlobalModuleConfig;
}


int ModuleManager::initModule()
{
    if (LsiapiBridge::initLsiapi() != 0)
        return LS_FAIL;

    memset(m_aModuleDataCount, 0, sizeof(short) * LSI_DATA_COUNT);
    clear();
    return 0;
}


int ModuleManager::getModulePath(const char *name, char *path, int max_len)
{
    int n = snprintf(path, max_len - 1, "%s/modules/%s.so",
                     MainServerConfig::getInstance().getServerRoot(),
                     name);
    if (n >= max_len)
    {
        n = max_len - 1;
        path[n] = 0;
    }
    return n;
}


// int ModuleManager::testModuleCount(const XmlNodeList *pList)
// {
//     int count = 0;
//     char sFilePath[512];
//     const char *name;
//     void *sLib;
//     lsi_module_t *pModule;
//     const XmlNode *pModuleNode;
//     XmlNodeList::const_iterator iter;
//     for( iter = pList->begin(); iter != pList->end(); ++iter )
//     {
//         pModuleNode = *iter;
//         name = pModuleNode->getChildValue( "name" );
//         getModulePath(pModuleNode->getChildValue( "name" ), sFilePath);
//         sLib = dlopen ( sFilePath, RTLD_LAZY );
//         if ( sLib )
//         {
//             pModule = ( lsi_module* ) dlsym ( sLib, name );
//             if ( dlerror() == NULL && pModule )
//                 ++count;
//             dlclose(sLib);
//         }
//     }
//     return count;
// }


int ModuleManager::storeModulePointer(int index, lsi_module_t *pModule)
{
    static int module_count = INITIAL_MODULE_COUNT;
    if (module_count <= index)
    {
        module_count += 5;
        m_pModuleArray = (ModulePointer *)realloc(m_pModuleArray,
                         module_count * sizeof(ModulePointer));
    }
    m_pModuleArray[index] = pModule;
    return 0;
}


ModuleManager::iterator ModuleManager::addModule(const char *name,
        const char *pType, lsi_module_t *pModule)
{
    iterator iter;
    LsiModule *pLmHttpHandler = new LsiModule(pModule);
    MODULE_NAME(pModule) = strdup(name);
    MODULE_ID(pModule) = getModuleCount();
    MODULE_LOG_LEVEL(pModule) = *g_api->_log_level_ptr;
    memset(MODULE_DATA_ID(pModule), 0xFF,
           sizeof(short) * LSI_DATA_COUNT); //value is -1 now.
    MODULE_HOOKINDEX(pModule) = new ModIndex();
    MODULE_HANDLER(pModule) = pLmHttpHandler;
    //m_gModuleArray[pModule->_id] = pModule;
    storeModulePointer(MODULE_ID(pModule), pModule);

    iter = insert((char *) MODULE_NAME(pModule), pLmHttpHandler);
    LS_DBG_H("%s module [%s] built with API v%hd.%hd has been"
             " registered successfully.",
             pType, MODULE_NAME(pModule),
             (int16_t)(pModule->signature >> 16),
             (int16_t)pModule->signature);

    if (strcmp(name, "modpagespeed") == 0)
    {
        //Loaded modpagespeed,
        HttpServerConfig::getInstance().setUsePagespeed(1);
    }

    return iter;
}


//extern lsi_module_t * getPrelinkedModule( const char * pModule );
extern int getPrelinkedModuleCount();
extern lsi_module_t *getPrelinkedModuleByIndex(unsigned int index,
        const char **pName);


int ModuleManager::loadPrelinkedModules()
{
    int n = getPrelinkedModuleCount();
    lsi_module_t *pModule;
    int count = 0;
    const char *pName;
    for (int i = 0; i < n; ++i)
    {
        pModule = getPrelinkedModuleByIndex(i, &pName);
        if (pModule)
        {
            if (addModule(pName, "Internal", pModule) != end())
            {
                ModuleConfig::parsePriority(NULL, MODULE_PRIORITY(pModule));
                ++count;
            }
        }
    }
    return count;
}


lsi_module_t *ModuleManager::loadModule(const char *name)
{
    const char *error;
    lsi_module_t *pModule = NULL;
    void *dlLib = NULL;
    const char *pType = "External";

    iterator iter = find(name);
    if (iter != end())
    {
        //loaded, can be internal or just loaded external
        if (strcmp(name, "cache") != 0)
        {
            error = "Module loaded, cannot load multiple times.";
            LS_ERROR("Failed to load module [%s], error: %s", name, error);
        }
        return NULL;
    }

    //prelinked checked, now, check file
    char sFilePath[512];
    getModulePath(name, sFilePath, sizeof(sFilePath));
    dlLib = dlopen(sFilePath, RTLD_LAZY | RTLD_GLOBAL);
    if (dlLib)
        pModule = (lsi_module_t *) dlsym(dlLib, name);
    if (!pModule || !dlLib)
        error = dlerror();

    if (pModule)
    {
        if (pModule->signature != LSI_MODULE_SIGNATURE)
        {
            if ((pModule->signature >> 32) != (LSI_MODULE_SIGNATURE >> 32))
                error = "Module signature does not match";
            else
                error = "Module version mismatch";
        }
        else
        {
            if (addModule(name, pType, pModule) != end())
                return pModule;
            else
                error = "Registration failure.";
        }
    }

    LS_ERROR("Failed to load module [%s], error: %s", name, error);

    if (dlLib)
        dlclose(dlLib);

    return NULL;
}


void ModuleManager::disableModule(lsi_module_t *pModule)
{
    int i;
    pModule->reqhandler = NULL;

    for (i = 0; i < LSI_HKPT_TOTAL_COUNT; ++i)
        ((LsiApiHooks *)(LsiApiHooks::getGlobalApiHooks(i)))->remove(pModule);

}


static void checkModuleDef(lsi_module_t *pModule)
{
#define DEF_VERSION_INFO_REPFIX  "version:"
#define DEF_VERSION_INFO_REPFIX_SIZE  (sizeof(DEF_VERSION_INFO_REPFIX) -1)


    char defPath[513] = {0};
    snprintf(defPath, 512, "%s/modules/%s.def",
             MainServerConfig::getInstance().getServerRoot(),
             MODULE_NAME(pModule));
    const char *info = pModule->about;
    int match = 0;
    size_t infoLen = 0;
    if (info)
    {
        infoLen = strlen(info);
        //We will save and compare only the first 240 bytes
        if (infoLen > 240)
            infoLen = 240;
    }


    FILE *fp = fopen(defPath, "rb");
    if (fp)
    {
        char sLine[256] = {0};
        fgets(sLine, 255, fp);
        size_t len2 = strlen(sLine);
        if (len2 > DEF_VERSION_INFO_REPFIX_SIZE)
        {
            char *pinfo = sLine + DEF_VERSION_INFO_REPFIX_SIZE;
            len2 -= DEF_VERSION_INFO_REPFIX_SIZE + 1;

            // We added a \n at the end of the line, so adjust for it.
            if (len2 >= 1)
                len2 -= 1;

            if (info == NULL)
            {
                if (len2 == 0)
                    match = 1;
            }
            else
            {
                if (infoLen == len2 &&
                    memcmp(info, pinfo, infoLen) == 0)
                    match = 1;
            }
        }
        fclose(fp);
    }

    if (!match)
    {
        fp = fopen(defPath, "wb");
        if (fp)
        {
            AutoStr2 sInfo;
            sInfo.setStr(DEF_VERSION_INFO_REPFIX);
            if (info != NULL)
                sInfo.append(info, infoLen);

            fprintf(fp, "%s\n\n", sInfo.c_str());

            if (pModule->config_parser)
            {
                lsi_config_key_t *p = pModule->config_parser->config_keys;
                while (p->config_key != NULL)
                {
                    fprintf(fp, "param.%s\t%d\t%d\n",
                            p->config_key,
                            p->id,
                            p->level);
                    ++p;
                }
            }

            if (pModule->serverhook)
            {
                int count = 0;
                while (pModule->serverhook[count].cb)
                {
                    int index = pModule->serverhook[count].index;
                    if (index < 0 || index >= LSI_HKPT_TOTAL_COUNT)
                    {
                        LS_ERROR("[%s] create def file failure,"
                                 " wrong index(%d).\n",
                                 MODULE_NAME(pModule), index);
                    }
                    else
                        fprintf(fp, "priority.%s:%d\n",
                                LsiApiHooks::s_pHkptName[index],
                                pModule->serverhook[count].priority);
                    ++count;
                }
            }
            fclose(fp);
        }
    }
}


int ModuleManager::runModuleInit()
{
    lsi_module_t *pModule;
    for (int i = 0; i < getModuleCount(); ++i)
    {
        pModule = m_pModuleArray[i];
        checkModuleDef(pModule);

        if (pModule->init_pf)
        {
            int ret = pModule->init_pf(pModule);
            if (ret != 0)
            {
                disableModule(pModule);
                LS_ERROR("[%s] initialization failure, disabled",
                         MODULE_NAME(pModule));
                continue;
            }
        }

        //add global level hooks here
        if (pModule->serverhook)
        {
            int count = 0;
            while (pModule->serverhook[count].cb)
            {
                add_global_hook(pModule->serverhook[count].index,
                                pModule,
                                pModule->serverhook[count].cb,
                                pModule->serverhook[count].priority,
                                pModule->serverhook[count].flag);
                ++count;
            }
        }

        LS_INFO("[Module: %s %s] has been initialized successfully",
                MODULE_NAME(pModule),
                ((pModule->about) ? pModule->about : ""));
    }

    return 0;
}


int ModuleManager::loadModules(const XmlNodeList *pModuleNodeList)
{
    m_pModuleArray = (ModulePointer *)malloc(INITIAL_MODULE_COUNT * sizeof(
                         ModulePointer));
    XmlNodeList::const_iterator iter;
    int count = loadPrelinkedModules();
    if (!pModuleNodeList)
        return count;

    for (iter = pModuleNodeList->begin(); iter != pModuleNodeList->end();
         ++iter)
    {
        const XmlNode *pModuleNode = *iter;
        lsi_module_t *pModule;
        const char *name = pModuleNode->getChildValue("name", 1);
        if (!name)
            continue;
        if ((pModule = loadModule(name)) != NULL)
        {
            ModuleConfig::parsePriority(pModuleNode, MODULE_PRIORITY(pModule));
            ++count;
        }
    }

    return count;
}


int ModuleManager::unloadModules()
{
    LsiModule *pLmHttpHandler;
    iterator iter;
    for (iter = begin(); iter != end(); iter = next(iter))
    {
        pLmHttpHandler = (LsiModule *) iter.second();
        free((void *)MODULE_NAME(pLmHttpHandler->getModule()));
        delete pLmHttpHandler;
        pLmHttpHandler = NULL;
    }
    clear();
    free(m_pModuleArray);
    return 0;
}


short ModuleManager::getModuleDataCount(unsigned int level)
{
    if (level < LSI_DATA_COUNT)
        return m_aModuleDataCount[level];
    else
        return LS_FAIL;//Error, should not access this value
}


void ModuleManager::incModuleDataCount(unsigned int level)
{
    if (level < LSI_DATA_COUNT)
        ++ m_aModuleDataCount[level];
}


void ModuleManager::OnTimer10sec()
{
    //LsiapiBridge::checkExpiredGData();
}


void ModuleManager::OnTimer100msec()
{
    ModTimerList::getInstance().checkExpired();
}


void ModuleManager::applyConfigToIolinkRt(IolinkSessionHooks
        *pSessionHooks, ModuleConfig *moduleConfig)
{
    int count = moduleConfig->getCount();
    for (short module_id = 0; module_id < count; ++module_id)
    {
        if (!moduleConfig->getFilterEnable(module_id))
            pSessionHooks->setModuleEnable(m_pModuleArray[module_id], 0);
    }
}


void ModuleManager::applyConfigToServerRt(ServerSessionHooks
        *pSessionHooks, ModuleConfig *moduleConfig)
{
    int count = moduleConfig->getCount();
    for (short module_id = 0; module_id < count; ++module_id)
    {
        if (!moduleConfig->getFilterEnable(module_id))
            pSessionHooks->setModuleEnable(m_pModuleArray[module_id], 0);
    }
}


void ModuleManager::applyConfigToHttpRt(HttpSessionHooks *pSessionHooks,
                                        ModuleConfig *moduleConfig)
{
    int count = moduleConfig->getCount();
    for (short module_id = 0; module_id < count; ++module_id)
    {
        if (!moduleConfig->getFilterEnable(module_id))
            pSessionHooks->setModuleEnable(m_pModuleArray[module_id], 0);
    }
}


void ModuleManager::updateHttpApiHook(HttpSessionHooks *pRtHooks,
                                      ModuleConfig *moduleConfig, int module_id)
{
    if (!moduleConfig)
        return ;
    if (!moduleConfig->getFilterEnable(module_id))
        pRtHooks->setModuleEnable(m_pModuleArray[module_id], 0);
    //FIXME: shoudl th ebelow code be called?
    //else
    //  pRtHooks->setModuleEnable(m_pModuleArray[module_id], 1);
}


void ModuleConfig::init(int count)
{
    bool need_init = initData(count);
    if (need_init)
    {
        for (int i = 0; i < m_iCount; ++i)
        {
            lsi_module_config_t *pConfig = new lsi_module_config_t;
            memset(pConfig, 0, sizeof(lsi_module_config_t));
            LsiModuleData::set(i, (void *) pConfig);
        }
    }
}


ModuleConfig::~ModuleConfig()
{
    lsi_module_config_t *config = NULL;
    for (int i = 0; i < m_iCount; ++i)
    {
        config = get(i);
        if ((config->data_flag == LSI_CONFDATA_PARSED)
            && (config->config != NULL)
            && (config->module->config_parser->free_config))
            config->module->config_parser->free_config(config->config);
        delete config;
    }
}


void ModuleConfig::setFilterEnable(short _module_id, int v)
{
    ModuleConfig::setFilterEnable(get(_module_id), v);
}


int ModuleConfig::getFilterEnable(short _module_id)
{
    return ModuleConfig::getFilterEnable(get(_module_id));
}


int ModuleConfig::compare(lsi_module_config_t *config1,
                          lsi_module_config_t *config2)
{
    if (config1->filters_enable == config2->filters_enable &&
        config1->module == config2->module)
        return 0;
    else
        return 1;
}


void ModuleConfig::inherit(const ModuleConfig *parentConfig)
{
    for (int i = 0; i < m_iCount; ++i)
    {
        lsi_module_config_t *config = get(i);
        memcpy(config, parentConfig->get(i), sizeof(lsi_module_config_t));
        config->data_flag = LSI_CONFDATA_NONE;
        config->sparam = NULL;
    }
}


//Return 0, no match. 1, match root
int ModuleConfig::isMatchGlobal()
{
    for (int i = 0; i < m_iCount; ++i)
    {
        if (getFilterEnable(i) == 0)
            return 0;
    }
    return 1;
}


int ModuleConfig::parsePriority(const XmlNode *pModuleNode, int *priority)
{
    const char *pHkptName;
    const char *pValue = NULL;
    for (int i = 0; i < LSI_HKPT_TOTAL_COUNT; ++i)
    {
        pHkptName = LsiApiHooks::s_pHkptName[i];
        if ((pModuleNode)
            && (pValue = pModuleNode->getChildValue(pHkptName)))
            priority[i] = atoi(pValue);
        else
            priority[i] = LSI_HOOK_PRIORITY_MAX + 1;
    }
    return 0;
}


int ModuleConfig::saveConfig(const XmlNode *pNode, lsi_module_t *pModule,
                             lsi_module_config_t *module_config)
{
    const char *pValue = NULL;
    assert(module_config->module == pModule);

//9/17/2018 diesable context level ls_enabled config, to avoid conflict
//    with vhost config
//     pValue = pNode->getChildValue("ls_enabled");
//     if (!pValue)
//         pValue = pNode->getChildValue("enabled");
//
//     if (pValue)
//         module_config->filters_enable = (int16_t)atoi(pValue);
//     else
//         module_config->filters_enable = -1;

    if (pModule->config_parser && pModule->config_parser->parse_config)
    {
        if ((pValue = pNode->getChildValue("param")) != NULL)
        {
            if (!module_config->sparam)
                module_config->sparam = new AutoStr2;

            if (module_config->sparam)
            {
                module_config->sparam->append("\n", 1);
                module_config->sparam->append(pValue, strlen(pValue));
                module_config->data_flag = LSI_CONFDATA_OWN;
            }
        }
    }
    else
    {
        module_config->sparam = NULL;
        module_config->data_flag = LSI_CONFDATA_NONE;
    }

    return 0;
}


// int ModuleConfig::parseOutsideModuleParam(const XmlNode *pNode)
// {
//     XmlNodeList::const_iterator iter;
//     const XmlNodeList *pUnknList = parentNode->getChildren("unknownkeywords");
//     if (!pUnknList )
//         return 0;
//
//     for( iter = pUnknList->begin(); iter != pUnknList->end(); ++iter )
//     {
//         config->config = pModule->config_parser->parse_config((*iter)->getValue(), config->config);
//
//
//     ModuleConfig *pConfig = new ModuleConfig;
//         pConfig->init(ModuleManager::getInstance().getModuleCount());
//         pConfig->inherit(ModuleManager::getGlobalModuleConfig());
//         ModuleConfig::parseConfigList(pModuleList, pConfig);
//         pRootContext->setModuleConfig(pConfig, 1);
//
// }


//Return the count of the keys items
static int checkConfigKeys(lsi_config_key_t *keys)
{
    lsi_config_key_t * p = keys;
    int index = 0;
    while(p->config_key)
    {
        if (p->id == 0)
            p->id = index++;
        else
            index = p->id + 1; //use for the next id if that one not assigned id

        if (p->level == 0)
            p->level = LSI_CFG_SERVER | LSI_CFG_LISTENER | LSI_CFG_VHOST | LSI_CFG_CONTEXT;
        ++p;
    }

    return p - keys;
}

int ModuleConfig::getKeyIndex(lsi_config_key_t *keys, const char *key, int key_len)
{
    for (int i=0;; ++i)
    {
        if (keys[i].config_key)
        {
            if (strncasecmp(keys[i].config_key, key, key_len) == 0)
                return i;
        }
        else
            break;
    }
    return -1;
}

void ModuleConfig::releaseModuleParamInfo(module_param_info_t *param_arr,
                                          int param_count)
{
    for (int i=0; i<param_count; ++i)
    {
        if (param_arr[i].val)
            free(param_arr[i].val);
    }
}


//val_out must have space at least len + 1(will append NULL at the end)
//return the val_out length
int ModuleConfig::escapeParamVal(const char *val_in, int len, char *val)
{
    char escapedChar = 0x00;
    int count =0;
    const char *pValStr = val_in;
    const char *valEnd = val_in + len;

    while(pValStr < valEnd)
    {
        switch (*pValStr)
        {
        case '`':
        case '\'':
        case '\"':
            if (escapedChar == 0x00)
                escapedChar = *pValStr;
            else if (escapedChar == *pValStr)
                escapedChar = 0x00;
            else
                val[count++] = *pValStr;
            break;

        case '\\':
            //If Last char(does not have the next char) copy it
            if (pValStr == valEnd -1 || escapedChar == 0x00)
                val[count++] = '\\';
            else
            {
                if (*(pValStr + 1) == escapedChar)
                {
                    ++pValStr;
                    val[count++] = escapedChar;
                }
                else
                    val[count++] = '\\';
            }
            break;

        default:
            val[count++] = *pValStr;
            break;
        }
        ++pValStr;
    }

    return count;
}

/**
 * ls_getconfkey return the key of the line,
 * Comments: key can not be multiple line and should be have space inside
 */
const char *ModuleConfig::ls_getconfkey(const char **pParseBegin,
                                        const char *pParseEnd,
                                        const char **pKeyEnd)
{
    ls_strtrim2(pParseBegin, &pParseEnd);
    const char *pKeyBegin = *pParseBegin;

    if (*pParseBegin < pParseEnd)
    {
        while(!isspace(**pParseBegin))
            ++ *pParseBegin;
    }

    *pKeyEnd = *pParseBegin;
    return pKeyBegin;
}


/**
 * ls_get_escconfval is used to get the conf value which may be in multiple line
 * and has quote charactor inside and may be have more than one items.
 * Comments: return the val length, and val need to be malloc'd first.
 */
int ModuleConfig::ls_get_escconfval(const char **pParseBegin,
                                    const char *pParseEnd,
                                    char *val)
{
    char escapedChar = 0x00;
    int count =0;
    const char *pValStr = *pParseBegin;

    while(pValStr < pParseEnd)
    {
        switch (*pValStr)
        {
        case '`':
        case '\'':
        case '\"':
            if (escapedChar == 0x00)
                escapedChar = *pValStr;
            else if (escapedChar == *pValStr)
                escapedChar = 0x00;
            else
                val[count++] = *pValStr;
            break;

        case '\\':
            //If Last char(does not have the next char) copy it
            if (pValStr == pParseEnd -1 || escapedChar == 0x00)
                val[count++] = '\\';
            else
            {
                if (*(pValStr + 1) == escapedChar)
                {
                    ++pValStr;
                    val[count++] = escapedChar;
                }
                else
                    val[count++] = '\\';
            }
            break;

        case '\r':
        case '\n':
            if (escapedChar == 0x00)
                pParseEnd = pValStr;
            else
                val[count++] = *pValStr;
            break;

        default:
            val[count++] = *pValStr;
            break;
        }
        ++pValStr;
    }

    *pParseBegin = pParseEnd + 1;
    return count;
}


/***
 * Server will parse module param for module, to better using this feature,
 * module need to provide the param lsi_config_key_s array, which includes
 * param key, param index, usage levels.
 * After parsing, module will get the parsed array, and will be easily to set to
 * its' own data.
 * During parsing, `, ' and " will be treat as quote, inside a quote,
 * such as `, \` will be treat as escape charactor and it will be `,
 * but won't escape other quote charactor, such as \", so
 *   \` 123 \" \` 456 `
 * will be parsed as
 *   \ 123 \" ` 456
 * And diffrent quoted string can be combined, such as
 *   \` 123 \" \` 456 `   " 789 \" \` 0 "
 * will be parsed as
 *   \ 123 \" ` 456     789 " \` 0
 * A special case is
 *   `123\\`345`
 * will be parsed as
 *   123\`345
 * For more details, please check our wiki.
 *
 */
int ModuleConfig::preParseModuleParam(const char *param, int paramLen,
                                      int level, lsi_config_key_t *keys,
                                      module_param_info_t *param_arr,
                                      int *param_count)
{
    int max_param_count = *param_count;
    const char *pKey;
    char pVal[8192] = {0};
    const char *pKeyEnd;
    const char *pConf = param;
    const char *pConfEnd = param + paramLen;
    int param_arr_sz = 0;

    while(pConf < pConfEnd)
    {
        pKey = ls_getconfkey(&pConf, pConfEnd, &pKeyEnd);
        if (pKey)
        {
            LS_DBG_H("Key:[%.*s]\n", (int)(pKeyEnd - pKey), pKey );

            while(*pConf == ' ' || *pConf == '\t')
                ++ pConf;

            int valLen = ls_get_escconfval(&pConf, pConfEnd, pVal);
            LS_DBG_H("Val:[%.*s]\n", valLen, pVal );

            int key_index = getKeyIndex(keys, pKey, pKeyEnd - pKey);
            if (key_index != -1)
            {
                if (keys[key_index].level & level)
                {
                    if (param_arr_sz < max_param_count)
                    {
                        param_arr[param_arr_sz].key_index = key_index;
                        param_arr[param_arr_sz].val_len = valLen;
                        param_arr[param_arr_sz].val = strndup(pVal, valLen);
                        ++param_arr_sz;
                    }
                    else
                    {
                        LS_ERROR("Error: preParseModuleParam get more items "
                            "than defined max count %d, have to give up.\n",
                            max_param_count);
                        break;
                    }
                }
            }
        }
    }

    *param_count = param_arr_sz;
    return 0;
}


int ModuleConfig::parseConfig(const XmlNode *pNode, lsi_module_t *pModule,
                              ModuleConfig *pModuleConfig, int level, const char *name)
{
    const char *pValue = NULL;
    int iValueLen = 0;

    int module_id = MODULE_ID(pModule);
    lsi_module_config_t *config = pModuleConfig->get(module_id);
    config->module = pModule;

    pValue = pNode->getChildValue("ls_enabled");
    if (!pValue)
        pValue = pNode->getChildValue("enabled");

    if (pValue)
        ModuleConfig::setFilterEnable(config, atoi(pValue));

    pValue = pNode->getChildValue("param");
    iValueLen = pNode->getChildValueLen("param");

    config->data_flag = LSI_CONFDATA_NONE;
    if (pModule->config_parser && pModule->config_parser->parse_config)
    {
        lsi_config_key_t *keys = pModule->config_parser->config_keys;
        if (keys)
        {
            //If the id/level not defined, changes to default values
            checkConfigKeys(keys);

            module_param_info_t param_arr[MAX_MODULE_CONFIG_LINES];
            int param_arr_sz = MAX_MODULE_CONFIG_LINES;
            preParseModuleParam(pValue, iValueLen, level, keys,
                                param_arr, &param_arr_sz);
            if (param_arr_sz > 0 || LSI_CFG_SERVER == level)
            {
                config->config = pModule->config_parser->parse_config(param_arr, param_arr_sz,
                                                     config->config, level, name);
                releaseModuleParamInfo(param_arr, param_arr_sz);
                config->data_flag = LSI_CONFDATA_PARSED;
            }
        }
    }
    config->sparam = NULL;
    return 0;
}


int ModuleConfig::parseConfigList(const XmlNodeList *moduleConfigNodeList,
                                  ModuleConfig *pModuleConfig, int level, const char *name)
{
    if (!moduleConfigNodeList)
        return 0;

    int ret = 0;
    const char *pValue = NULL;
    XmlNode *pNode = NULL;

    XmlNodeList::const_iterator iter;
    for (iter = moduleConfigNodeList->begin();
         iter != moduleConfigNodeList->end(); ++iter)
    {
        pNode = *iter;
        pValue = pNode->getChildValue("name", 1);
        if (!pValue)
        {
            ret = -1;
            LS_DBG_H("[LSIAPI] parseConfigList error, no module name");
            break;
        }

        ModuleManager::iterator moduleIter;
        moduleIter = ModuleManager::getInstance().find(pValue);
        if (moduleIter == ModuleManager::getInstance().end())
            continue;

        parseConfig(pNode, moduleIter.second()->getModule(), pModuleConfig,
                    level, name);
    }
    return ret;
}
