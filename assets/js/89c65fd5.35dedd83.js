"use strict";(self.webpackChunkcachelib=self.webpackChunkcachelib||[]).push([[5395],{3905:function(e,t,a){a.r(t),a.d(t,{MDXContext:function(){return l},MDXProvider:function(){return u},mdx:function(){return f},useMDXComponents:function(){return h},withMDXComponents:function(){return m}});var i=a(67294);function n(e,t,a){return t in e?Object.defineProperty(e,t,{value:a,enumerable:!0,configurable:!0,writable:!0}):e[t]=a,e}function r(){return r=Object.assign||function(e){for(var t=1;t<arguments.length;t++){var a=arguments[t];for(var i in a)Object.prototype.hasOwnProperty.call(a,i)&&(e[i]=a[i])}return e},r.apply(this,arguments)}function c(e,t){var a=Object.keys(e);if(Object.getOwnPropertySymbols){var i=Object.getOwnPropertySymbols(e);t&&(i=i.filter((function(t){return Object.getOwnPropertyDescriptor(e,t).enumerable}))),a.push.apply(a,i)}return a}function o(e){for(var t=1;t<arguments.length;t++){var a=null!=arguments[t]?arguments[t]:{};t%2?c(Object(a),!0).forEach((function(t){n(e,t,a[t])})):Object.getOwnPropertyDescriptors?Object.defineProperties(e,Object.getOwnPropertyDescriptors(a)):c(Object(a)).forEach((function(t){Object.defineProperty(e,t,Object.getOwnPropertyDescriptor(a,t))}))}return e}function s(e,t){if(null==e)return{};var a,i,n=function(e,t){if(null==e)return{};var a,i,n={},r=Object.keys(e);for(i=0;i<r.length;i++)a=r[i],t.indexOf(a)>=0||(n[a]=e[a]);return n}(e,t);if(Object.getOwnPropertySymbols){var r=Object.getOwnPropertySymbols(e);for(i=0;i<r.length;i++)a=r[i],t.indexOf(a)>=0||Object.prototype.propertyIsEnumerable.call(e,a)&&(n[a]=e[a])}return n}var l=i.createContext({}),m=function(e){return function(t){var a=h(t.components);return i.createElement(e,r({},t,{components:a}))}},h=function(e){var t=i.useContext(l),a=t;return e&&(a="function"==typeof e?e(t):o(o({},t),e)),a},u=function(e){var t=h(e.components);return i.createElement(l.Provider,{value:t},e.children)},d={inlineCode:"code",wrapper:function(e){var t=e.children;return i.createElement(i.Fragment,{},t)}},p=i.forwardRef((function(e,t){var a=e.components,n=e.mdxType,r=e.originalType,c=e.parentName,l=s(e,["components","mdxType","originalType","parentName"]),m=h(a),u=n,p=m["".concat(c,".").concat(u)]||m[u]||d[u]||r;return a?i.createElement(p,o(o({ref:t},l),{},{components:a})):i.createElement(p,o({ref:t},l))}));function f(e,t){var a=arguments,n=t&&t.mdxType;if("string"==typeof e||n){var r=a.length,c=new Array(r);c[0]=p;var o={};for(var s in t)hasOwnProperty.call(t,s)&&(o[s]=t[s]);o.originalType=e,o.mdxType="string"==typeof e?e:n,c[1]=o;for(var l=2;l<r;l++)c[l]=a[l];return i.createElement.apply(null,c)}return i.createElement.apply(null,a)}p.displayName="MDXCreateElement"},15424:function(e,t,a){a.r(t),a.d(t,{assets:function(){return m},contentTitle:function(){return s},default:function(){return d},frontMatter:function(){return o},metadata:function(){return l},toc:function(){return h}});var i=a(83117),n=a(80102),r=(a(67294),a(3905)),c=["components"],o={id:"compact_cache_design",title:"Compact Cache Design"},s=void 0,l={unversionedId:"Cache_Library_Architecture_Guide/compact_cache_design",id:"Cache_Library_Architecture_Guide/compact_cache_design",title:"Compact Cache Design",description:"What is Compact Cache?",source:"@site/docs/Cache_Library_Architecture_Guide/Compact_cache_design.md",sourceDirName:"Cache_Library_Architecture_Guide",slug:"/Cache_Library_Architecture_Guide/compact_cache_design",permalink:"/docs/Cache_Library_Architecture_Guide/compact_cache_design",draft:!1,editUrl:"https://github.com/facebook/CacheLib/edit/main/website/docs/Cache_Library_Architecture_Guide/Compact_cache_design.md",tags:[],version:"current",frontMatter:{id:"compact_cache_design",title:"Compact Cache Design"},sidebar:"archguideSideBar",previous:{title:"Slab Rebalancing - How Does It Work?",permalink:"/docs/Cache_Library_Architecture_Guide/slab_rebalancing"},next:{title:"Hybrid Cache",permalink:"/docs/Cache_Library_Architecture_Guide/hybrid_cache"}},m={},h=[{value:"What is Compact Cache?",id:"what-is-compact-cache",level:2},{value:"High level design",id:"high-level-design",level:2},{value:"Finding the bucket for an entry",id:"finding-the-bucket-for-an-entry",level:3},{value:"Reading an entry",id:"reading-an-entry",level:3},{value:"Writing an entry into compact cache",id:"writing-an-entry-into-compact-cache",level:3},{value:"Compact cache with variable size values",id:"compact-cache-with-variable-size-values",level:3},{value:"Comparison with regular items cache",id:"comparison-with-regular-items-cache",level:2},{value:"Possible future improvements",id:"possible-future-improvements",level:2}],u={toc:h};function d(e){var t=e.components,o=(0,n.Z)(e,c);return(0,r.mdx)("wrapper",(0,i.Z)({},u,o,{components:t,mdxType:"MDXLayout"}),(0,r.mdx)("h2",{id:"what-is-compact-cache"},"What is Compact Cache?"),(0,r.mdx)("p",null,"Compact Cache is  designed to cache small items with low overheads (0-4 bytes) compared to the regular items cache (32 bytes), making it space efficient when storing small items (tens of bytes). To use compact cache, the key size must be fixed at compile time  for a given instance of compact cache, and value size can be fixed or variable."),(0,r.mdx)("h2",{id:"high-level-design"},"High level design"),(0,r.mdx)("p",null,"Cache library's memory management component divides pre-allocated memory into 4MB pieces (slabs), and distribute them to each memory pools according to config. A compact cache instance leverages a memory pool to allocate memory in large chunks and caches smaller object using that."),(0,r.mdx)("p",null,(0,r.mdx)("img",{src:a(7471).Z,width:"684",height:"248"})),(0,r.mdx)("p",null,"Each slab is sliced into buckets, the size of each bucket is ",(0,r.mdx)("em",{parentName:"p"},"(key_size + value_size) ")," 8,* so the bucket is just an array of 8 key-value pair."),(0,r.mdx)("p",null,(0,r.mdx)("img",{src:a(88039).Z,width:"7350",height:"1450"})),(0,r.mdx)("h3",{id:"finding-the-bucket-for-an-entry"},"Finding the bucket for an entry"),(0,r.mdx)("p",null,"First, the key is hashed into a number (key_hash), and ",(0,r.mdx)("em",{parentName:"p"},"key_hash % num_of_slabs")," to identify the slab for the key_hash. Inside the slab, ",(0,r.mdx)("em",{parentName:"p"},"key_hash % num_of_buckets")," will find the right bucket. To transparently expand the capacity of the compact cache without losing all its contents, compact cache uses a consistent hashing scheme to map the key hash to slab."),(0,r.mdx)("h3",{id:"reading-an-entry"},"Reading an entry"),(0,r.mdx)("p",null,"After locating the right bucket for the key of entry,  the bucket is locked in shared mode and each entry is iterated in the bucket to match the full key. If a match is found, the value is copied and returned, and the entry is promoted  to the head of the bucket if the option is enabled."),(0,r.mdx)("h3",{id:"writing-an-entry-into-compact-cache"},"Writing an entry into compact cache"),(0,r.mdx)("p",null,"After locating the right bucket for the key of the entry, the bucket is locked in exclusive mode. Next, a scan is performed to see if key has  exists in the bucket. If the key exists, the value is replaced with the new one and  the entry is promoted to the head of the bucket. Otherwise the a suitable  entry is evicted from the tail of the bucket, and the new entry is inserted at the head of the bucket."),(0,r.mdx)("h3",{id:"compact-cache-with-variable-size-values"},"Compact cache with variable size values"),(0,r.mdx)("p",null,"Compact cache supports variable size values. In this mode, each bucket is fixed size and entries with various sizes are fitted into it by fitting the fixed size keys first and then using the remaining space for variable sized values."),(0,r.mdx)("p",null,(0,r.mdx)("img",{src:a(45632).Z,width:"9510",height:"1070"})),(0,r.mdx)("p",null,"Upon insertion, if there is not enough empty space in the middle, entries are evicted and the new entry is inserted."),(0,r.mdx)("h2",{id:"comparison-with-regular-items-cache"},"Comparison with regular items cache"),(0,r.mdx)("p",null,"By design, Compact cache is meant to be low or zero overhead, which is clearly an advantage from items cache, especially when the item key+value size is very small. It is advised to have key + value sizes be smaller than a cache line size to leverage the efficiency of compact cache.   But there are some limitations also:"),(0,r.mdx)("ul",null,(0,r.mdx)("li",{parentName:"ul"},"Size of entry has to be small, which will limit the use case of compact cache significantly. If the key/value size is too big, there will be two prominent negative performance impact on read:",(0,r.mdx)("ul",{parentName:"li"},(0,r.mdx)("li",{parentName:"ul"},"Searching for the right key will be expensive because the total size of keys in the bucket are way too big compared to the size of CPU cache line."),(0,r.mdx)("li",{parentName:"ul"},"Unlike items cache which can return a folly::IOBuf that holds a refcount on the item, compact cache always copy into the given buffer. This means when the value size is larger, the more inefficient the compact cache becomes."))),(0,r.mdx)("li",{parentName:"ul"},"LRU/FIFO is on per bucket basis",(0,r.mdx)("ul",{parentName:"li"},(0,r.mdx)("li",{parentName:"ul"},"Compact cache can only do LRU or FIFO inside the bucket, which can cause some imbalance between buckets. Some use cases can be sensitive to this (hit rate may be worse)."))),(0,r.mdx)("li",{parentName:"ul"},"Key size is static at compile time",(0,r.mdx)("ul",{parentName:"li"},(0,r.mdx)("li",{parentName:"ul"},"Currently Compact Cache does not support variable key size. There is a workaround by padding '\\0' bytes at the tail of smaller keys to make all keys at the same biggest size, but obviously this wastes memory, one of the primary motivations for using compact cache."))),(0,r.mdx)("li",{parentName:"ul"},"Compact cache size needs to be tuned periodically as workloads change."),(0,r.mdx)("li",{parentName:"ul"},"There is no native TTL support"),(0,r.mdx)("li",{parentName:"ul"},"Compact Cache has totally different set of interface compared to items cache")),(0,r.mdx)("h2",{id:"possible-future-improvements"},"Possible future improvements"),(0,r.mdx)("p",null,"Some explorative ideas to improve the trade-off benefits of compact cache"),(0,r.mdx)("ul",null,(0,r.mdx)("li",{parentName:"ul"},"Fast entry locating",(0,r.mdx)("ul",{parentName:"li"},(0,r.mdx)("li",{parentName:"ul"},"This can be achieved by hashing the keys into one byte each and storing them at the head of the bucket."),(0,r.mdx)("li",{parentName:"ul"},"With  GCC built-in Functions for SSE2/SSE3 Instructions, we can locate keys efficiently instead of comparing them one by one."))),(0,r.mdx)("li",{parentName:"ul"},"Automatically rebalance the memory pool size between items cache and compact Cache",(0,r.mdx)("ul",{parentName:"li"},(0,r.mdx)("li",{parentName:"ul"},"If the resizing can be automated with some metric and strategy setup similar to rebalance between allocation classes inside a memory pool, this will be a huge improvement on performance tuning"))),(0,r.mdx)("li",{parentName:"ul"},"TTL support"),(0,r.mdx)("li",{parentName:"ul"},"Performance test",(0,r.mdx)("ul",{parentName:"li"},(0,r.mdx)("li",{parentName:"ul"},"A helpful setup to identify the break-even key/value sizes for compact cache vs items cache."))),(0,r.mdx)("li",{parentName:"ul"},"Supporting variable key sizes",(0,r.mdx)("ul",{parentName:"li"},(0,r.mdx)("li",{parentName:"ul"},"Current implementation of variable value size is by putting all values at the right side of bucket buffer. We can extend this design by storing the keys at the right side of bucket buffer together with the values"))),(0,r.mdx)("li",{parentName:"ul"},"Unified API for compact cache and items cache, there are couple of options:",(0,r.mdx)("ul",{parentName:"li"},(0,r.mdx)("li",{parentName:"ul"},"Have a higher level of API wrapping the CacheAllocator inside that contains one (may be able to support multiple) regular items pool, one compact cache for small items and another compact cache for negative keys. The API can be as simple as:",(0,r.mdx)("ul",{parentName:"li"},(0,r.mdx)("li",{parentName:"ul"},"get: find the key from all caches"),(0,r.mdx)("li",{parentName:"ul"},"set: put an entry into regular items cache or small item Compact cache depends on entry size, and invalidate the key from other caches"),(0,r.mdx)("li",{parentName:"ul"},"setNegative: put an entry into Negative Cache, and invalidate the key from other caches"),(0,r.mdx)("li",{parentName:"ul"},"invalidate: invalidate a given key from all caches"),(0,r.mdx)("li",{parentName:"ul"},"We can also serialize same-key operation by having built-in lock maps, which was requested by several clients and this feature is missing in cache library"))),(0,r.mdx)("li",{parentName:"ul"},"Have the same set of interface for compact cache as items cache in cache allocator, like find, allocate, etc. And cache allocator figure out the right place to find/allocate the entry")))))}d.isMDXComponent=!0},7471:function(e,t,a){t.Z=a.p+"assets/images/Compact_Cache_Design_cc_contain_slabs-c77d4385e438dc51cb850b301b110ddf.png"},45632:function(e,t,a){t.Z=a.p+"assets/images/Compact_Cache_Design_cc_veriable-7348b7aaf5228f6694eb8a0748a0b136.png"},88039:function(e,t,a){t.Z=a.p+"assets/images/Compact_Cache_Design_image-838daaa60c334094b95a9fcb66338466.png"}}]);