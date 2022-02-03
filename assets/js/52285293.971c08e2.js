"use strict";(self.webpackChunkmy_website=self.webpackChunkmy_website||[]).push([[9980],{3905:function(e,t,n){n.d(t,{Zo:function(){return m},kt:function(){return d}});var i=n(7294);function s(e,t,n){return t in e?Object.defineProperty(e,t,{value:n,enumerable:!0,configurable:!0,writable:!0}):e[t]=n,e}function o(e,t){var n=Object.keys(e);if(Object.getOwnPropertySymbols){var i=Object.getOwnPropertySymbols(e);t&&(i=i.filter((function(t){return Object.getOwnPropertyDescriptor(e,t).enumerable}))),n.push.apply(n,i)}return n}function l(e){for(var t=1;t<arguments.length;t++){var n=null!=arguments[t]?arguments[t]:{};t%2?o(Object(n),!0).forEach((function(t){s(e,t,n[t])})):Object.getOwnPropertyDescriptors?Object.defineProperties(e,Object.getOwnPropertyDescriptors(n)):o(Object(n)).forEach((function(t){Object.defineProperty(e,t,Object.getOwnPropertyDescriptor(n,t))}))}return e}function a(e,t){if(null==e)return{};var n,i,s=function(e,t){if(null==e)return{};var n,i,s={},o=Object.keys(e);for(i=0;i<o.length;i++)n=o[i],t.indexOf(n)>=0||(s[n]=e[n]);return s}(e,t);if(Object.getOwnPropertySymbols){var o=Object.getOwnPropertySymbols(e);for(i=0;i<o.length;i++)n=o[i],t.indexOf(n)>=0||Object.prototype.propertyIsEnumerable.call(e,n)&&(s[n]=e[n])}return s}var r=i.createContext({}),c=function(e){var t=i.useContext(r),n=t;return e&&(n="function"==typeof e?e(t):l(l({},t),e)),n},m=function(e){var t=c(e.components);return i.createElement(r.Provider,{value:t},e.children)},u={inlineCode:"code",wrapper:function(e){var t=e.children;return i.createElement(i.Fragment,{},t)}},p=i.forwardRef((function(e,t){var n=e.components,s=e.mdxType,o=e.originalType,r=e.parentName,m=a(e,["components","mdxType","originalType","parentName"]),p=c(n),d=s,g=p["".concat(r,".").concat(d)]||p[d]||u[d]||o;return n?i.createElement(g,l(l({ref:t},m),{},{components:n})):i.createElement(g,l({ref:t},m))}));function d(e,t){var n=arguments,s=t&&t.mdxType;if("string"==typeof e||s){var o=n.length,l=new Array(o);l[0]=p;var a={};for(var r in t)hasOwnProperty.call(t,r)&&(a[r]=t[r]);a.originalType=e,a.mdxType="string"==typeof e?e:s,l[1]=a;for(var c=2;c<o;c++)l[c]=n[c];return i.createElement.apply(null,l)}return i.createElement.apply(null,n)}p.displayName="MDXCreateElement"},6027:function(e,t,n){n.r(t),n.d(t,{frontMatter:function(){return a},contentTitle:function(){return r},metadata:function(){return c},toc:function(){return m},default:function(){return p}});var i=n(7462),s=n(3366),o=(n(7294),n(3905)),l=["components"],a={title:"Testing",id:"testing"},r="Testing CacheLib",c={unversionedId:"installation/testing",id:"installation/testing",title:"Testing",description:"Cachelib includes many unit tests for various components",source:"@site/docs/installation/testing.md",sourceDirName:"installation",slug:"/installation/testing",permalink:"/docs/installation/testing",editUrl:"https://github.com/facebook/docusaurus/edit/master/website/docs/installation/testing.md",tags:[],version:"current",frontMatter:{title:"Testing",id:"testing"},sidebar:"installationSidebar",previous:{title:"Installation",permalink:"/docs/installation/"},next:{title:"Github Repository Synchronization (after cleanup)",permalink:"/docs/installation/github-squash-sync"}},m=[{value:"Building CacheLib Unit Tests",id:"building-cachelib-unit-tests",children:[],level:2},{value:"Running individual unit tests",id:"running-individual-unit-tests",children:[],level:2},{value:"Running all  unit tests",id:"running-all--unit-tests",children:[],level:2}],u={toc:m};function p(e){var t=e.components,n=(0,s.Z)(e,l);return(0,o.kt)("wrapper",(0,i.Z)({},u,n,{components:t,mdxType:"MDXLayout"}),(0,o.kt)("h1",{id:"testing-cachelib"},"Testing CacheLib"),(0,o.kt)("p",null,(0,o.kt)("inlineCode",{parentName:"p"},"Cachelib")," includes many unit tests for various components\nof the cache infrastructure."),(0,o.kt)("h2",{id:"building-cachelib-unit-tests"},"Building CacheLib Unit Tests"),(0,o.kt)("p",null,"To build the cachelib unit tests, use one of the following commands\n(see ",(0,o.kt)("a",{parentName:"p",href:"docs/installation/installation"},"installation")," instructions for more details):"),(0,o.kt)("ol",null,(0,o.kt)("li",{parentName:"ol"},"Use ",(0,o.kt)("inlineCode",{parentName:"li"},"./contrib/build.sh")," script with the ",(0,o.kt)("inlineCode",{parentName:"li"},"-T")," option."),(0,o.kt)("li",{parentName:"ol"},"Use ",(0,o.kt)("inlineCode",{parentName:"li"},"./contrib/build-package.sh -t cachelib")," (with the ",(0,o.kt)("inlineCode",{parentName:"li"},"-t")," option)")),(0,o.kt)("p",null,"The unit test binaries will be installed in ",(0,o.kt)("inlineCode",{parentName:"p"},"./opt/cachelib/tests"),":"),(0,o.kt)("pre",null,(0,o.kt)("code",{parentName:"pre",className:"language-sh"},"$ git clone https://github.com/facebook/CacheLib\n$ cd CacheLib\n$ ./contrib/build.sh -T -j\n$ cd opt/cachelib/tests\n$ ls\nallocator-test-AllocationClassTest\nallocator-test-AllocatorHitStatsTypeTest\nallocator-test-AllocatorResizeTypeTest\nallocator-test-AllocatorTypeTest\n...\nnavy-test-ThreadPoolJobSchedulerTest\nnavy-test-UtilsTest\nshm-test-test_page_size\nshm-test-test_posix\n")),(0,o.kt)("h2",{id:"running-individual-unit-tests"},"Running individual unit tests"),(0,o.kt)("p",null,"Running a single unit test binary:"),(0,o.kt)("pre",null,(0,o.kt)("code",{parentName:"pre",className:"language-sh"},"$ cd opt/cachelib/tests\n$ ./allocator-test-ItemTest \n[==========] Running 6 tests from 1 test suite.\n[----------] Global test environment set-up.\n[----------] 6 tests from ItemTest\n[ RUN      ] ItemTest.NonStringKey\n[       OK ] ItemTest.NonStringKey (0 ms)\n[ RUN      ] ItemTest.CreationTime\n[       OK ] ItemTest.CreationTime (0 ms)\n[ RUN      ] ItemTest.ExpiryTime\n[       OK ] ItemTest.ExpiryTime (0 ms)\n[ RUN      ] ItemTest.ChainedItemConstruction\n[       OK ] ItemTest.ChainedItemConstruction (0 ms)\n[ RUN      ] ItemTest.ChangeKey\n[       OK ] ItemTest.ChangeKey (0 ms)\n[ RUN      ] ItemTest.ToString\n[       OK ] ItemTest.ToString (0 ms)\n[----------] 6 tests from ItemTest (0 ms total)\n[----------] Global test environment tear-down\n[==========] 6 tests from 1 test suite ran. (0 ms total)\n[  PASSED  ] 6 tests.\n")),(0,o.kt)("h2",{id:"running-all--unit-tests"},"Running all  unit tests"),(0,o.kt)("p",null,"Running ",(0,o.kt)("inlineCode",{parentName:"p"},"make")," in the unit tests directory will execute all tests.\nUse ",(0,o.kt)("inlineCode",{parentName:"p"},"make -j")," to utilize all available CPUs."),(0,o.kt)("pre",null,(0,o.kt)("code",{parentName:"pre",className:"language-sh"},"$ cd opt/cachelib/tests\n$ nice make -j\nRunning common-test-BloomFilterTest\nRunning common-test-AccessTrackerTest\nRunning common-test-BloomFilterTest\nRunning common-test-BytesEqualTest\nRunning common-test-CohortTests\n...\n")),(0,o.kt)("p",null,"Result of each test is stored in a ",(0,o.kt)("inlineCode",{parentName:"p"},".log")," file:"),(0,o.kt)("pre",null,(0,o.kt)("code",{parentName:"pre",className:"language-sh"},"$ cat common-test-BloomFilterTest.log\n[==========] Running 11 tests from 1 test suite.\n[----------] Global test environment set-up.\n[----------] 11 tests from BloomFilter\n[ RUN      ] BloomFilter.OptimalParams\n[       OK ] BloomFilter.OptimalParams (1397 ms)\n[ RUN      ] BloomFilter.Default\n[       OK ] BloomFilter.Default (0 ms)\n[ RUN      ] BloomFilter.Move\n[       OK ] BloomFilter.Move (0 ms)\n[ RUN      ] BloomFilter.Reset\n[       OK ] BloomFilter.Reset (0 ms)\n[ RUN      ] BloomFilter.SimpleCollision\n[       OK ] BloomFilter.SimpleCollision (0 ms)\n[ RUN      ] BloomFilter.SharedCollision\n[       OK ] BloomFilter.SharedCollision (0 ms)\n[ RUN      ] BloomFilter.InvalidArgs\n[       OK ] BloomFilter.InvalidArgs (0 ms)\n[ RUN      ] BloomFilter.Clear\n[       OK ] BloomFilter.Clear (0 ms)\n[ RUN      ] BloomFilter.PersistRecoverWithInvalidParams\n[       OK ] BloomFilter.PersistRecoverWithInvalidParams (2 ms)\n[ RUN      ] BloomFilter.PersistRecoveryValidLarge\n[       OK ] BloomFilter.PersistRecoveryValidLarge (4599 ms)\n[ RUN      ] BloomFilter.PersistRecoveryValid\n[       OK ] BloomFilter.PersistRecoveryValid (1 ms)\n[----------] 11 tests from BloomFilter (6002 ms total)\n\n[----------] Global test environment tear-down\n[==========] 11 tests from 1 test suite ran. (6002 ms total)\n[  PASSED  ] 11 tests.\n")),(0,o.kt)("p",null,"A summary log file is generated (",(0,o.kt)("inlineCode",{parentName:"p"},"cachelib-test-summary.log"),") containing test counts,\nand logs of all the ",(0,o.kt)("em",{parentName:"p"},"failed")," tests:"),(0,o.kt)("pre",null,(0,o.kt)("code",{parentName:"pre",className:"language-sh"},"$ cd opt/cachelib/tests\n$ cat cachelib-test-summary.log\n=== Cachelib Test Summary ===\n\n107 TESTS PASSED\n3 TESTS FAILED:\n  allocator-test-NvmCacheTests\n  common-test-TimeTests\n  shm-test-test_page_size\n\n(system information at the end of this file)\n\n\n=== FAILED TEST: allocator-test-NvmCacheTests ===\n\nRunning main() from /home/assafgordon/cachelib-oss-ci/CacheLib/cachelib/external/googletest/googletest/src/gtest_main.cc\n[==========] Running 34 tests from 1 test suite.\n[----------] Global test environment set-up.\n[----------] 34 tests from NvmCacheTest\n[ RUN      ] NvmCacheTest.Config\nI0809 06:00:30.146022 1106342 Factory.cpp:272] Cache file: /tmp/nvmcache-cachedir/1106342/navy size: 104857600 truncate: 0\nI0809 06:00:30.152341 1106342 NavySetup.cpp:138] metadataSize: 4194304 bigHashCacheOffset: 52428800 bigHashCacheSize: 52428800\nI0809 06:00:30.152387 1106342 NavySetup.cpp:166] blockcache: starting offset: 4194304, block cache size: 46137344\nI0809 06:00:30.152500 1106342 LruPolicy.cpp:21] LRU policy: expected 11 regions\nI0809 06:00:30.158718 1106342 RegionManager.cpp:31] 11 regions, 4194304 bytes each\n...\n")),(0,o.kt)("p",null,"Sharing the ",(0,o.kt)("inlineCode",{parentName:"p"},"cachelib-test-summary.log")," file with the CacheLib developers can help us diagnose and fix problems."))}p.isMDXComponent=!0}}]);