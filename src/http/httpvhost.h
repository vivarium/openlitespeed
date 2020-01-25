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
#ifndef HTTPVHOST_H
#define HTTPVHOST_H

#include <http/contexttree.h>
#include <http/httpcontext.h>
#include <http/httplogsource.h>
#include <http/reqstats.h>
#include <http/rewriterulelist.h>
#include <http/throttlecontrol.h>
#include <http/reqparserparam.h>
#include <log4cxx/nsdefs.h>
#include <lsiapi/lsimoduledata.h>

#include <util/hashstringmap.h>
#include <util/refcounter.h>
#include <util/stringlist.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <extensions/registry/extappregistry.h>


BEGIN_LOG4CXX_NS
class Logger;
class Appender;
END_LOG4CXX_NS

#define LS_NEVER_FOLLOW     0
#define LS_ALWAYS_FOLLOW    1
#define LS_FOLLOW_OWNER     2
#define VH_SYM_CTRL         3
#define VH_ENABLE           4
#define VH_SERVER_ENABLE    8
#define VH_ENABLE_SCRIPT    16
//#define VH_CONTEXT_AC       32
#define VH_RESTRAINED       64
#define VH_ACC_LOG          128
#define VH_GZIP             256
#define VH_BR               512
#define VH_AUTOLOADHTACCESS  1024
#define VH_CGROUP           2048
#define VH_RECAPTCHA        4096

#define MAX_VHOST_PHP_NUM    100


#define DETACH_MODE_MIN_MAX_IDLE 30



#define DEFAULT_ADMIN_SERVER_NAME   "_AdminVHost"

typedef struct lsi_module_s lsi_module_t;
class AccessCache;
class AccessControl;
class AccessLog;
class Awstats;
class ConfigCtx;
class Env;
class ExpiresCtrl;
class FcgiApp;
class HashDataCache;
class HotlinkCtrl;
class HTAuth;
class HttpHandler;
class HttpMime;
class HttpServerImpl;
class HttpSession;
class LocalWorker;
class LocalWorkerConfig;
class LsiApiHooks;
class ModUserdir;
class Recaptcha;
class RewriteMapList;
class RLimits;
class SsiTagConfig;
class SslContext;
class UserDir;
class XmlNodeList;
class ExtWorker;

template< class T >
class THash;
class StaticFileCacheData;

#define MAX_ACCESS_LOG    4


struct php_xml_st
{
    AutoStr suffix;
    AutoStr app_name;
    XmlNode *xml_node;
};

class RealmMap : public HashStringMap< UserDir * >
{
    typedef HashStringMap< UserDir * > _shmap;
public:
    explicit RealmMap(int initSize = 20);
    ~RealmMap();
    const UserDir *find(const char *pScript) const;
    UserDir *get(const char *pFile, const char *pGroup);
    RealmMap(const RealmMap &rhs);
    void operator=(const RealmMap &rhs);
};

typedef struct _static_file_data
{
    AutoStr2 url;
    time_t tmaccess;
    StaticFileCacheData *pData;
} static_file_data_t;

typedef  THash<static_file_data_t *> UrlStxFileHash;


typedef struct _url_id_data
{
    AutoStr2 url;
    u_int32_t  id;
} url_id_data_t;

/**
 * UrlIdHash define a url ---> id mapping hash
 */
typedef  THash<url_id_data_t *> UrlIdHash;

class HttpVHost : public RefCounter, public HttpLogSource
{
private:
    ReqStats            m_reqStats;
    AccessLog          *m_pAccessLog[MAX_ACCESS_LOG];
    LOG4CXX_NS::Logger *m_pLogger;
    LOG4CXX_NS::Appender *m_pBytesLog;

    ThrottleLimits      m_throttle;

    int16_t             m_iMaxKeepAliveRequests;

    int                 m_iFeatures;

    AccessCache        *m_pAccessCache;
    ContextTree         m_contexts;
    HttpContext         m_rootContext;
    AutoStr2            m_sVhRoot;
    HotlinkCtrl        *m_pHotlinkCtrl;
    RealmMap            m_realmMap;
    StringList          m_matchNameList;

    Awstats            *m_pAwstats;

    AutoStr2            m_sName;
    AutoStr2            m_sAdminEmails;
    AutoStr2            m_sAutoIndexURI;

    int                 m_iMappingRef;

    uid_t               m_uid;
    gid_t               m_gid;
    char                m_iRewriteLogLevel;
    char                m_iGlobalMatchContext;
    AutoStr2            m_sChroot;
    RewriteMapList     *m_pRewriteMaps;
    SslContext         *m_pSSLCtx;
    SsiTagConfig       *m_pSSITagConfig;
    LsiModuleData       m_moduleData;
    Recaptcha          *m_pRecaptcha;
    AccessLog          *m_lastAccessLog;

    UrlStxFileHash     *m_pUrlStxFileHash;
    php_xml_st          m_pPhpXmlNodeS[MAX_VHOST_PHP_NUM];
    int                 m_PhpXmlNodeSSize;

    UrlIdHash          *m_pUrlIdHash;

    HttpVHost(const HttpVHost &rhs);
    void operator=(const HttpVHost &rhs);




public:
    explicit HttpVHost(const char *pHostName);
    ~HttpVHost();


    int setDocRoot(const char *psRoot);
    const AutoStr2 *getDocRoot() const
    {   return m_rootContext.getRoot();     }

    const char *getName() const            {   return m_sName.c_str();     }
    void updateName(const char *p)       {   m_sName.setStr(p);        }

    void setVhRoot(const char *psVhRoot)  {   m_sVhRoot = psVhRoot;       }
    const AutoStr2 *getVhRoot() const      {   return &m_sVhRoot;          }

//     const StringList* getIndexFileList() const
//     {   return &m_indexFileList;  }
//     StringList* getIndexFileList()          {   return &m_indexFileList;    }
//     void setIndexFileList(StringList * p)   {   m_rootContext.setIndexFileList( p );    }

    ReqStats *getReqStats()                {   return &m_reqStats;         }


//    int  setCustomErrUrls(int statusCode, const char* url)
//    {   return m_rootContext.setCustomErrUrls( statusCode, url );  }
//    const AutoStr2 * getErrDocUrl( int statusCode ) const
//    {   return m_rootContext.getErrDocUrl( statusCode );    }

    const char *getAccessLogPath() const;
    LOG4CXX_NS::Logger *getLogger() const  {   return m_pLogger;       }

    AccessControl *getAccessCtrl() const;
    AccessCache *getAccessCache() const    {   return m_pAccessCache;  }

    HotlinkCtrl *getHotlinkCtrl() const    {   return m_pHotlinkCtrl;  }
    void setHotlinkCtrl(HotlinkCtrl *p)  {   m_pHotlinkCtrl = p;     }

    UserDir *getRealm(const char *pRealm);
    const UserDir *getRealm(const char *pRealm) const;
    void onTimer();
    void onTimer30Secs();

    //const char * getAdminEmails() const;
    const AutoStr2 *getAdminEmails() const {   return &m_sAdminEmails;  }
    void setAdminEmails(const char *pEmails);

    int addContext(HttpContext *pContext)
    {   return m_contexts.add(pContext);          }

    bool dirMatch(HttpContext * &pContext, const char *pURI, size_t iUriLen,
                  AutoStr2 *missingDir, AutoStr2 *missLoc) const;
    HttpContext *bestMatch(const char *pURI, size_t iUriLen);

    const HttpContext *matchLocation(const char *pURI, size_t iUriLen,
                                     int regex = 0) const;

    HttpContext *getContext(const char *pURI, size_t iUriLen,
                            int regex = 0) const;

    ContextTree *getContextTree()
    {   return &m_contexts;     }

    virtual void setLogLevel(const char *pLevel);
    virtual int  setAccessLogFile(const char *pFileName, int pipe);
    virtual int  setErrorLogFile(const char *pFileName);
    virtual void setErrorLogRollingSize(off_t size, int keep_days);

    virtual AccessLog *getAccessLog() const
    {   return m_lastAccessLog ? m_lastAccessLog : m_pAccessLog[0];    }

    void offsetChroot(const char *pChroot, int len);

    void setFeature(int bit, int enable)
    {   m_iFeatures = (m_iFeatures & (~bit)) | ((enable) ? bit : 0);     }

    int  isEnabled() const  {   return m_iFeatures & VH_ENABLE;         }
    void enable(int enable)
    {   setFeature(VH_ENABLE, (m_iFeatures & VH_SERVER_ENABLE) ? enable : 0);  }

    void serverEnabled(int enable)     {   setFeature(VH_SERVER_ENABLE, enable); }

    //void enableScript( int enable )     {   setFeature( VH_ENABLE_SCRIPT, enable ); }
    //int  isScriptEnabled() const        {   return m_iFeatures & VH_ENABLE_SCRIPT;  }
    void enableScript(int enable)     {   m_rootContext.enableScript(enable); }
    int  isScriptEnabled() const        {   return m_rootContext.isScriptEnabled();  }

    void followSymLink(int follow)
    {   m_iFeatures = (m_iFeatures & (~VH_SYM_CTRL)) | (follow & VH_SYM_CTRL); }
    int  followSymLink() const          {   return m_iFeatures & VH_SYM_CTRL;   }

    void enableAccessCtrl();

    void restrained(int enable)       {   setFeature(VH_RESTRAINED, enable);  }
    int  restrained() const             {   return m_iFeatures & VH_RESTRAINED; }

    void enableAccessLog(int enable)  {   setFeature(VH_ACC_LOG, enable); }
    int  enableAccessLog() const        {   return m_iFeatures & VH_ACC_LOG;}

    void enableGzip(int enable)       {   setFeature(VH_GZIP, enable);    }
    int  enableGzip() const             {   return m_iFeatures & VH_GZIP;   }

    void enableAutoLoadHt(int enable)       {   setFeature(VH_AUTOLOADHTACCESS, enable);    }
    int  enableAutoLoadHt() const             {   return m_iFeatures & VH_AUTOLOADHTACCESS;   }

    void enableBr(int enable)         {   setFeature(VH_BR, enable);      }
    int  enableBr() const               {   return m_iFeatures & VH_BR;     }

    void enableCGroup(int enable)       {   setFeature(VH_CGROUP, enable);    }
    int  enableCGroup() const             {   return m_iFeatures & VH_CGROUP;   }

    ExpiresCtrl &getExpires()           {   return m_rootContext.getExpires();  }
    const ExpiresCtrl &getExpires() const
    {   return m_rootContext.getExpires();           }

    HttpMime *getMIME()                {   return m_rootContext.getMIME();     }
    const HttpMime *getMIME() const    {   return m_rootContext.getMIME();     }

    HttpContext &getRootContext()              {   return m_rootContext;   }
    const HttpContext &getRootContext() const   {   return m_rootContext;   }

    void  logAccess(HttpSession *pSession) const;

    const AutoStr2 *addMatchName(const char *pName)
    {
        const AutoStr2 *ret = m_matchNameList.find(pName);
        if (!ret)
            return m_matchNameList.add(pName);
        else
            return ret;
    }
    UserDir *getFileUserDir(const char *pName,
                            const char *pFile, const char *pGroup);

    void contextInherit()
    {   m_contexts.contextInherit();     }

    int getPhpXmlNodeSSize() { return m_PhpXmlNodeSSize; }
    php_xml_st *getPhpXmlNodeS(int index)
    {
        if (index < m_PhpXmlNodeSSize)
            return &m_pPhpXmlNodeS[index];
        else
            return NULL;
    }
    int addPhpXmlNodeSSize(char *suffix, char *appName, XmlNode *pNode)
    {
        if(m_PhpXmlNodeSSize >= MAX_VHOST_PHP_NUM - 1)
            return -1;
        m_pPhpXmlNodeS[m_PhpXmlNodeSSize].suffix.setStr(suffix);
        m_pPhpXmlNodeS[m_PhpXmlNodeSSize].app_name.setStr(appName);
        m_pPhpXmlNodeS[m_PhpXmlNodeSSize].xml_node = pNode;
        ++m_PhpXmlNodeSSize;
        return 0;
    }

    /**
     * For the VHost UniqueAppName and UniqueAppUri we do the same way
     * add ".$uid" to the end of the string
     */

    void getUniAppName(const char *app_name, char *dst, int dst_len) const
    {    ExtAppRegistry::getUniAppUri(app_name, dst, dst_len, getUid()); }

    void setUid(uid_t uid)    {   m_uid = uid;    }
    uid_t getUid() const        {   return m_uid;   }

    void setGid(gid_t gid)    {   m_gid = gid;    }
    gid_t getGid() const        {   return m_gid;   }

    void incMappingRef()        {   ++m_iMappingRef;        }
    void decMappingRef()        {   --m_iMappingRef;        }
    int getMappingRef() const   {   return m_iMappingRef;   }

    void setChroot(const char *pRoot);
    const AutoStr2 *getChroot() const  {   return &m_sChroot;      }

    void setUidMode(int a)    {   m_rootContext.setUidMode(a);      }
    void setChrootMode(int a) {   m_rootContext.setChrootMode(a);   }

    void setMaxKAReqs(int a)  {   m_iMaxKeepAliveRequests = a;        }
    short getMaxKAReqs() const  {   return m_iMaxKeepAliveRequests;     }

    ThrottleLimits *getThrottleLimits()    {   return &m_throttle;     }
    const ThrottleLimits *getThrottleLimits() const
    {   return &m_throttle;         }

    char getRewriteLogLevel() const     {   return m_iRewriteLogLevel;  }
    void setRewriteLogLevel(int l)    {   m_iRewriteLogLevel = l;     }

    const RewriteMapList *getRewriteMaps() const
    {   return m_pRewriteMaps;      }

    void addRewriteMap(const char *pName, const char *pLocation);
    void addRewriteRule(char *pRules);

    void updateUGid(const char *pLogId, const char *pPath);

    const AutoStr2 *getAutoIndexURI() const
    {   return &m_sAutoIndexURI;    }

    void setAutoIndexURI(const char *pURI)
    {   m_sAutoIndexURI.setStr(pURI);     }

    HttpContext *addContext(const char *pUri, int type,
                            const char *pLocation, const char *pHandler, int allowBrowse);
    HttpContext *setContext(HttpContext *pContext, const char *pUri,
                            int type, const char *pLocation, const char *pHandler,
                            int allowBrowse, int match = 0);

    void setAwstats(Awstats *pAwstats)
    {
        m_pAwstats = pAwstats;
    }

    void logBytes(long long bytes);
    void setBytesLogFilePath(const char *pPath, off_t rollingSize);
    int  BytesLogEnabled() const     {   return m_pBytesLog != NULL; }



    char isGlobalMatchContext() const           {   return m_iGlobalMatchContext;   }
    void setGlobalMatchContext(char global)   {   m_iGlobalMatchContext = global; }

    const char *getVhName(int &len) const
    {
        len = m_sName.len();
        return m_sName.c_str();
    }
    void setSsiTagConfig(SsiTagConfig *pConfig)
    {   m_pSSITagConfig = pConfig;  }

    SsiTagConfig *getSsiTagConfig() const
    {   return m_pSSITagConfig;     }
    void setSslContext(SslContext *pCtx);

    SslContext *getSslContext() const
    {   return m_pSSLCtx;           }

    HTAuth *configAuthRealm(HttpContext *pContext,
                            const char *pRealmName);
    int configContextAuth(HttpContext *pContext,
                          const XmlNode *pContextNode);
    int configBasics(const XmlNode *pVhConfNode, int iChrootLen);
    int configUserGroup(const XmlNode *pVhConfNode);
    int configWebsocket(const XmlNode *pWebsocketNode);
    int configVHWebsocketList(const XmlNode *pVhConfNode);
    int configHotlinkCtrl(const XmlNode *pNode);
    int setAuthCache(const XmlNode *pNode,
                     HashDataCache *pAuth);
    int configRealm(const XmlNode *pRealmNode);
    int configRealmList(const XmlNode *pRoot);
    int configSecurity(const XmlNode *pVhConfNode);
    int configRewrite(const XmlNode *pNode);
    void configRewriteMap(const XmlNode *pNode);
    HttpContext *addContext(int match, const char *pUri,
                            int type, const char *pLocation, const char *pHandler, int allowBrowse);
    HttpContext *configContext(const char *pUri, int type,
                               const char *pLocation,
                               const char *pHandler, int allowBrowse);
    const XmlNode *configIndex(const XmlNode *pVhConfNode,
                               const StringList *pStrList);
    int configIndexFile(const XmlNode *pVhConfNode,
                        const StringList *pStrList, const char *strIndexURI);
    int configAwstats(const char *vhDomain, int vhAliasesLen,
                      const XmlNode *pNode);

    void setDefaultConfig(LocalWorkerConfig &config,
                          const char *pBinPath, int maxConns,
                          int maxIdle, LocalWorkerConfig *pDefault);
    int testAppType(const char *pAppRoot);

    int configRailsRunner(char *pRunnerCmd, int cmdLen,
                                 const char *pRubyBin);

    LocalWorker *addRailsApp(const char *pAppName, const char *appPath,
                            int maxConns, const char *pRailsEnv,
                            int maxIdle, const Env *pEnv,
                            int runOnStart, const char *pBinPath);
    HttpContext *configRailsContext(const char *contextUri,
                                   const char *appPath,
                                   int maxConns, const char *pRailsEnv,
                                   int maxIdle, const Env *pEnv,
                                   const char *pBinPath,
                                   HttpContext *pOldCtx);

    HttpContext *addRailsContext(const char *pURI, const char *pLocation,
                                        ExtWorker *pWorker,
                                        HttpContext *pOldCtx);

    LocalWorker *addPythonApp(const char *pAppName,
                              const char *appPath, const char *pStartupFile,
                              int maxConns, const char *pPythonEnv, int maxIdle,
                              const Env *pEnv, int runOnStart,
                              const char *pBinPath);

    HttpContext *addPythonContext(const char *pURI,
                                  const char *pLocation,
                                  const char *pStartupFile, ExtWorker *pWorker,
                                  HttpContext *pOldCtx);


    int configNodeJsStarter(char *pRunnerCmd, int cmdLen, const char *pBinPath);
    LocalWorker *addNodejsApp(const char *pAppName,
                              const char *appPath, const char *pStartupFile,
                              int maxConns, const char *pRunModeEnv,
                              int maxIdle, const Env *pEnv, int runOnStart,
                              const char *pBinPath);
    HttpContext *addNodejsContext(const char *pURI,
                                  const char *pLocation,
                                  const char *pStartupFile, ExtWorker *pWorker,
                                  HttpContext *pOldCtx);


    HttpContext *configAppContext(const XmlNode *pNode,
                                  const char *contextUri, const char *appPath);
    HttpContext *configAppContext(int appType,
                                  const char *contextUri, const char *pAppRoot,
                                  const char *pStartupFile, int maxConns,
                                  const char * pAppMode, int maxIdle,
                                  const Env *pProcessEnv, int runOnStartup,
                                  const char *pBinPath, HttpContext *pOldCtx);


    HttpContext *importWebApp(const char *contextUri, const char *appPath,
                              const char *pWorkerName, int allowBrowse);
    int configContext(const XmlNode *pContextNode);

    void checkAndAddNewUriFormModuleList(const XmlNodeList *pModuleList);
    int configVHContextList(const XmlNode *pVhConfNode,
                            const XmlNodeList *pModuleList);
    int configVHModuleUrlFilter1(lsi_module_t *pModule,
                                 const XmlNodeList *pfilterList);  //step 1 just save params
    int configVHModuleUrlFilter2(lsi_module_t *pModule,
                                 const XmlNodeList *pfilterList);  //step 2 parse the params
    int configModuleConfigInContext(const XmlNode *pContextNode,
                                    int saveParam);

    int parseVHModulesParams(const XmlNode *pVhConfNode,
                             const XmlNodeList *pModuleList, int saveParam);

    int config(const XmlNode *pVhConfNode, int is_uid_set);
    int getExtAppGUid(const XmlNode *pExtAppNode);

    int configVHScriptHandler2();
    int configVHScriptHandler(const XmlNode *pVhConfNode);
    const HttpHandler *isHandlerAllowed(const HttpHandler *pHdlr, int type,
                                        const char *pHandler);
    void configVHChrootMode(const XmlNode *pNode);

    int configListenerMappings(const char *pListeners,
                               const char *pDomain,
                               const char *pAliases);

    static HttpVHost *configVHost(const XmlNode *pNode, const char *pName,
                                  const char *pDomain, const char *pAliases, const char *pVhRoot,
                                  const XmlNode *pConfigNode);
    static HttpVHost *configVHost(XmlNode *pNode);
    void configServletMapping(XmlNode *pRoot, char *achURI, int uriLen,
                              const char *pWorkerName, int allowBrowse);
    int configRedirectContext(const XmlNode *pContextNode,
                              const char *pLocation,
                              const char *pUri, const char *pHandler, bool allowBrowse, int match,
                              int type);
    int checkDeniedSubDirs(const char *pUri, const char *pLocation);

    LsiModuleData *getModuleData()      {   return &m_moduleData;   }

    void enableAioLogging();

    void addUrlStaticFileMatch(StaticFileCacheData *pData, const char *url, int urlLen);
    int checkFileChanged(static_file_data_t *data, struct stat &sb);
    void removeurlStaticFile(static_file_data_t *data);
    static_file_data_t *getUrlStaticFileData(const char *url);
    void urlStaticFileHashClean();

    int addUrlToUrlIdHash(const char *url);

    /**
     * return the bit of the url added to the hash
     */
    int getIdBitOfUrl(const char *url);

    int isRecaptchaEnabled() const      {   return m_iFeatures & VH_RECAPTCHA;  }
    const Recaptcha *getRecaptcha() const   {   return m_pRecaptcha;        }
    void setRecaptcha(Recaptcha *p)         {   m_pRecaptcha = p;           }
    int configRecaptcha(const XmlNode *pNode);
};


#endif
