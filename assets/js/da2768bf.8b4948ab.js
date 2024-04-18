"use strict";(self.webpackChunkcachelib=self.webpackChunkcachelib||[]).push([[9174],{3905:function(e,a,n){n.r(a),n.d(a,{MDXContext:function(){return m},MDXProvider:function(){return h},mdx:function(){return f},useMDXComponents:function(){return s},withMDXComponents:function(){return c}});var t=n(67294);function r(e,a,n){return a in e?Object.defineProperty(e,a,{value:n,enumerable:!0,configurable:!0,writable:!0}):e[a]=n,e}function i(){return i=Object.assign||function(e){for(var a=1;a<arguments.length;a++){var n=arguments[a];for(var t in n)Object.prototype.hasOwnProperty.call(n,t)&&(e[t]=n[t])}return e},i.apply(this,arguments)}function o(e,a){var n=Object.keys(e);if(Object.getOwnPropertySymbols){var t=Object.getOwnPropertySymbols(e);a&&(t=t.filter((function(a){return Object.getOwnPropertyDescriptor(e,a).enumerable}))),n.push.apply(n,t)}return n}function l(e){for(var a=1;a<arguments.length;a++){var n=null!=arguments[a]?arguments[a]:{};a%2?o(Object(n),!0).forEach((function(a){r(e,a,n[a])})):Object.getOwnPropertyDescriptors?Object.defineProperties(e,Object.getOwnPropertyDescriptors(n)):o(Object(n)).forEach((function(a){Object.defineProperty(e,a,Object.getOwnPropertyDescriptor(n,a))}))}return e}function d(e,a){if(null==e)return{};var n,t,r=function(e,a){if(null==e)return{};var n,t,r={},i=Object.keys(e);for(t=0;t<i.length;t++)n=i[t],a.indexOf(n)>=0||(r[n]=e[n]);return r}(e,a);if(Object.getOwnPropertySymbols){var i=Object.getOwnPropertySymbols(e);for(t=0;t<i.length;t++)n=i[t],a.indexOf(n)>=0||Object.prototype.propertyIsEnumerable.call(e,n)&&(r[n]=e[n])}return r}var m=t.createContext({}),c=function(e){return function(a){var n=s(a.components);return t.createElement(e,i({},a,{components:n}))}},s=function(e){var a=t.useContext(m),n=a;return e&&(n="function"==typeof e?e(a):l(l({},a),e)),n},h=function(e){var a=s(e.components);return t.createElement(m.Provider,{value:a},e.children)},p={inlineCode:"code",wrapper:function(e){var a=e.children;return t.createElement(t.Fragment,{},a)}},u=t.forwardRef((function(e,a){var n=e.components,r=e.mdxType,i=e.originalType,o=e.parentName,m=d(e,["components","mdxType","originalType","parentName"]),c=s(n),h=r,u=c["".concat(o,".").concat(h)]||c[h]||p[h]||i;return n?t.createElement(u,l(l({ref:a},m),{},{components:n})):t.createElement(u,l({ref:a},m))}));function f(e,a){var n=arguments,r=a&&a.mdxType;if("string"==typeof e||r){var i=n.length,o=new Array(i);o[0]=u;var l={};for(var d in a)hasOwnProperty.call(a,d)&&(l[d]=a[d]);l.originalType=e,l.mdxType="string"==typeof e?e:r,o[1]=l;for(var m=2;m<i;m++)o[m]=n[m];return t.createElement.apply(null,o)}return t.createElement.apply(null,n)}u.displayName="MDXCreateElement"},79789:function(e,a,n){n.r(a),n.d(a,{assets:function(){return c},contentTitle:function(){return d},default:function(){return p},frontMatter:function(){return l},metadata:function(){return m},toc:function(){return s}});var t=n(83117),r=n(80102),i=(n(67294),n(3905)),o=["components"],l={id:"synchronization_in_eviction_and_slab_rebalancing",title:"Synchronization between Eviction & Slab-movement"},d=void 0,m={unversionedId:"Cache_Library_Architecture_Guide/synchronization_in_eviction_and_slab_rebalancing",id:"Cache_Library_Architecture_Guide/synchronization_in_eviction_and_slab_rebalancing",title:"Synchronization between Eviction & Slab-movement",description:"Overview",source:"@site/docs/Cache_Library_Architecture_Guide/Synchronization_In_Eviction_And_Slab_Rebalacing.md",sourceDirName:"Cache_Library_Architecture_Guide",slug:"/Cache_Library_Architecture_Guide/synchronization_in_eviction_and_slab_rebalancing",permalink:"/docs/Cache_Library_Architecture_Guide/synchronization_in_eviction_and_slab_rebalancing",draft:!1,editUrl:"https://github.com/facebook/CacheLib/edit/main/website/docs/Cache_Library_Architecture_Guide/Synchronization_In_Eviction_And_Slab_Rebalacing.md",tags:[],version:"current",frontMatter:{id:"synchronization_in_eviction_and_slab_rebalancing",title:"Synchronization between Eviction & Slab-movement"},sidebar:"archguideSideBar",previous:{title:"Slab Rebalancing - How Does It Work?",permalink:"/docs/Cache_Library_Architecture_Guide/slab_rebalancing"},next:{title:"Compact Cache Design",permalink:"/docs/Cache_Library_Architecture_Guide/compact_cache_design"}},c={},s=[{value:"Overview",id:"overview",level:2},{value:"Regular Items",id:"regular-items",level:2},{value:"Chained Items",id:"chained-items",level:2}],h={toc:s};function p(e){var a=e.components,l=(0,r.Z)(e,o);return(0,i.mdx)("wrapper",(0,t.Z)({},h,l,{components:a,mdxType:"MDXLayout"}),(0,i.mdx)("h2",{id:"overview"},"Overview"),(0,i.mdx)("p",null,"There are two paths in CacheAllocator that manages the item memory ownership: eviction & slab-movement. When the cache is full, a user\u2019s allocation will trigger an eviction of an item to free up space for the new allocation. When CacheLib decides to move a slab, we will attempt to move or evict all the items that are currently active in the slab. We won\u2019t discuss why slab movement is necessary. Instead we focus on how synchronization is handled between these two code-paths in CacheAllocator."),(0,i.mdx)("p",null,"Eviction and slab-movement synchronize on an item\u2019s exclusive bit. For chained items, the synchronization happens on its parent item\u2019s exclusive bit."),(0,i.mdx)("p",null,"What is the exclusive bit? This refers to one of the bits in an item\u2019s header. A CacheLib item (cachelib/allocator/CacheItem.h) looks like the following."),(0,i.mdx)("pre",null,(0,i.mdx)("code",{parentName:"pre"},"---------------------\n| Intrusive Hooks   |\n| Reference & Flags |\n| Creation Time     |\n| Expiry Time       |\n| Payload           |\n---------------------\n")),(0,i.mdx)("p",null,"Item header refers to all fields above Payload. An item\u2019s ownership management happens within the \u201cReference & Flags\u201d field (cachelib/allocator/Refcount.h). Its purpose is as follows."),(0,i.mdx)("pre",null,(0,i.mdx)("code",{parentName:"pre"},"  /**\n   * Layout of refcount type. This includes the flags, admin ref bits, and\n   * refcount. Admin ref encodes special reference counts that contain\n   * information on who holds the reference count (like AccessContainer or\n   * MMContainer). Flags encode item state such as whether or not it's\n   * a chained item.\n   *\n   * The layout is as follows.\n   * |-- flags --|-- admin ref --|-- access ref--|\n   * - Flags:  11 bits\n   * - Admin Ref: 3 bits\n   * - Access Ref: 18 bits\n   */\n")),(0,i.mdx)("p",null,"The exclusive bit (kExclusive) and isInMMContainer (kLinked) are what we use to synchronize on an item for moving and eviction. They\u2019re part of the Admin Ref. When we mention refcounts, we typically mean Access Ref. These 18 bits are used to count how many outstanding handles there\u2019re for a given item."),(0,i.mdx)("p",null,(0,i.mdx)("strong",{parentName:"p"},"The following table shows when an item can enter into one of the following states.")),(0,i.mdx)("table",null,(0,i.mdx)("tr",null,(0,i.mdx)("td",null),(0,i.mdx)("td",null,"Unlinked (not yet inserted in cache)"),(0,i.mdx)("td",null,"Linked & Refcount > 0 (being accessed by someone else)"),(0,i.mdx)("td",null,"Linked & Refcount == 0 (currently not being accessed by anyone)"),(0,i.mdx)("td",null,"Exclusive"),(0,i.mdx)("td",null,"Moving")),(0,i.mdx)("tr",null,(0,i.mdx)("td",null,"incRef"),(0,i.mdx)("td",null,"Yes"),(0,i.mdx)("td",null,"Yes"),(0,i.mdx)("td",null,"Yes"),(0,i.mdx)("td",null,"No"),(0,i.mdx)("td",null,"No")),(0,i.mdx)("tr",null,(0,i.mdx)("td",null,"markForEviction"),(0,i.mdx)("td",null,"No"),(0,i.mdx)("td",null,"No"),(0,i.mdx)("td",null,"Yes"),(0,i.mdx)("td",null,"No"),(0,i.mdx)("td",null,"No")),(0,i.mdx)("tr",null,(0,i.mdx)("td",null,"markMoving"),(0,i.mdx)("td",null,"No"),(0,i.mdx)("td",null,"No"),(0,i.mdx)("td",null,"Yes"),(0,i.mdx)("td",null,"No"),(0,i.mdx)("td",null,"No"))),(0,i.mdx)("h2",{id:"regular-items"},"Regular Items"),(0,i.mdx)("p",null,"There are three ways a regular item\u2019s life-cycle is affected when it is in DRAM cache."),(0,i.mdx)("ol",null,(0,i.mdx)("li",{parentName:"ol"},"User or an internal cachelib thread ",(0,i.mdx)("strong",{parentName:"li"},"acquiring")," a refcount on an item. (incRef)"),(0,i.mdx)("li",{parentName:"ol"},"User or an internal cachelib thread marking an item for ",(0,i.mdx)("strong",{parentName:"li"},"eviction"),". (markForEviction)"),(0,i.mdx)("li",{parentName:"ol"},"An internal cachelib thread marking an item for ",(0,i.mdx)("strong",{parentName:"li"},"moving"),". (markMoving)")),(0,i.mdx)("p",null,(0,i.mdx)("img",{alt:"alt_text",src:n(9827).Z,title:"synchronization_in_eviction_and_slab_rebalancing_regular_item",width:"556",height:"822"})),(0,i.mdx)("p",null,"Let\u2019s now look at all the different scenarios the above operations can interact with one another."),(0,i.mdx)("p",null,(0,i.mdx)("strong",{parentName:"p"},"Eviction racing against Acquire")),(0,i.mdx)("ol",null,(0,i.mdx)("li",{parentName:"ol"},"Thread-A: user attempting to acquire an item handle for Foobar"),(0,i.mdx)("li",{parentName:"ol"},"Thread-B: user attempting to evict Foobar to make space for another allocation"),(0,i.mdx)("li",{parentName:"ol"},"{Thread-A | Thread-B} wins",(0,i.mdx)("ol",{parentName:"li"},(0,i.mdx)("li",{parentName:"ol"},"Thread-A wins. Thread-B will skip Foobar and look for another item for eviction."),(0,i.mdx)("li",{parentName:"ol"},"Thread-B wins. Thread-A will get a cache miss. This is fine because Foobar is now guaranteed to be freed and will not be visible to any users.")))),(0,i.mdx)("p",null,(0,i.mdx)("strong",{parentName:"p"},"Moving racing against Acquire")),(0,i.mdx)("ol",null,(0,i.mdx)("li",{parentName:"ol"},"Thread-A: user attempting to acquire an item handle for Foobar"),(0,i.mdx)("li",{parentName:"ol"},"Thread-B: cachelib internal thread attempting to move Foobar to another memory location"),(0,i.mdx)("li",{parentName:"ol"},"{Thread A | Thread-B} wins",(0,i.mdx)("ol",{parentName:"li"},(0,i.mdx)("li",{parentName:"ol"},"Thread-A wins. Thread-B will retry indefinitely until it\u2019s able to mark Foobar as moving, or if markMoving returns false which indicates the item had already been freed. The latter scenario can happen if Thread-A acquires the item handle first, removes it from the cache, and then drops the handle (if refcount == 0, the item will then be freed)."),(0,i.mdx)("li",{parentName:"ol"},"Thread-B wins. Thread-A will get an async ItemHandle back. This handle will become available when Foobar is successfully moved to another memory location, or evicted in which case the handle will return a cache miss.")))),(0,i.mdx)("p",null,(0,i.mdx)("strong",{parentName:"p"},"Eviction racing against Moving")),(0,i.mdx)("ol",null,(0,i.mdx)("li",{parentName:"ol"},"Thread-A: cachelib internal thread attempting to move Foobar to another memory location"),(0,i.mdx)("li",{parentName:"ol"},"Thread-B: user attempting to evict Foobar to make space for another allocation"),(0,i.mdx)("li",{parentName:"ol"},"{Thread A | Thread-B} wins",(0,i.mdx)("ol",{parentName:"li"},(0,i.mdx)("li",{parentName:"ol"},"Thread-A wins. Thread-B will skip Foobar and look for another item for eviction."),(0,i.mdx)("li",{parentName:"ol"},"Thread-B wins. Thread-A will retry until Foobar is evicted, after which point markMoving will return false, indicating the item had already been freed. Thread-B will skip this item and proceed to move the next item in the slab.")))),(0,i.mdx)("p",null,(0,i.mdx)("strong",{parentName:"p"},"Eviction, Moving, Acquire all racing against each other")),(0,i.mdx)("ol",null,(0,i.mdx)("li",{parentName:"ol"},"Thread-A: user attempting to acquire an item handle for Foobar"),(0,i.mdx)("li",{parentName:"ol"},"Thread-B: cachelib internal thread attempting to move Foobar to another memory location"),(0,i.mdx)("li",{parentName:"ol"},"Thread-C: user attempting to evict Foobar to make space for another allocation"),(0,i.mdx)("li",{parentName:"ol"},"{Thread A | Thread B | Thread C} wins",(0,i.mdx)("ol",{parentName:"li"},(0,i.mdx)("li",{parentName:"ol"},"Thread-A loses to C. User gets a cache miss."),(0,i.mdx)("li",{parentName:"ol"},"Thread-A loses to B. User gets a wait-context backed ItemHandle. Handle will be ready after B completes."),(0,i.mdx)("li",{parentName:"ol"},"Thread-B loses. Retry indefinitely until Foobar can be marked moving or it had already been freed."),(0,i.mdx)("li",{parentName:"ol"},"Thread-C loses. Skip Foobar and look for another item for eviction.")))),(0,i.mdx)("h2",{id:"chained-items"},"Chained Items"),(0,i.mdx)("p",null,"There are five ways a chained item\u2019s life-cycle is affected when it is in DRAM cache."),(0,i.mdx)("ol",null,(0,i.mdx)("li",{parentName:"ol"},"User or an internal cachelib thread ",(0,i.mdx)("strong",{parentName:"li"},"acquiring")," a refcount on an item."),(0,i.mdx)("li",{parentName:"ol"},"User or an internal cachelib thread marking an item for ",(0,i.mdx)("strong",{parentName:"li"},"eviction"),"."),(0,i.mdx)("li",{parentName:"ol"},"An internal cachelib thread marking an item for ",(0,i.mdx)("strong",{parentName:"li"},"moving"),".")),(0,i.mdx)("p",null,"First three are very similar to regular item with two key differences:"),(0,i.mdx)("ul",null,(0,i.mdx)("li",{parentName:"ul"},(0,i.mdx)("strong",{parentName:"li"},"The synchronization point of a chained item\u2019s lifecycle is through its parent."),(0,i.mdx)("ul",{parentName:"li"},(0,i.mdx)("li",{parentName:"ul"},"The above operations can happen on different chained items belonging to the same chain. But they will still race against one another via the parent of the chained item."))),(0,i.mdx)("li",{parentName:"ul"},(0,i.mdx)("strong",{parentName:"li"},"They also synchronize on the key\u2019s corresponding chained item lock."))),(0,i.mdx)("p",null,(0,i.mdx)("img",{alt:"alt_text",src:n(80319).Z,title:"synchronization_in_eviction_and_slab_rebalancing_chained_item_1",width:"768",height:"974"})),(0,i.mdx)("p",null,"The last two scenarios are new just for chained items. Instead of the parent item\u2019s exclusive bit, they synchronize on the key\u2019s corresponding chained item lock."),(0,i.mdx)("ol",{start:4},(0,i.mdx)("li",{parentName:"ol"},"User ",(0,i.mdx)("strong",{parentName:"li"},"adding/removing/replacing")," a chained item for the same parent item."),(0,i.mdx)("li",{parentName:"ol"},"User or an internal cachelib thread ",(0,i.mdx)("strong",{parentName:"li"},"transferring ownership")," of the chain to a new parent item.")),(0,i.mdx)("p",null,(0,i.mdx)("img",{alt:"alt_text",src:n(83925).Z,title:"synchronization_in_eviction_and_slab_rebalancing_chained_item_2",width:"1152",height:"856"})),(0,i.mdx)("p",null,"Let\u2019s now look at all the different scenarios the above operations can interact with one another."),(0,i.mdx)("p",null,(0,i.mdx)("strong",{parentName:"p"},"Eviction racing against Acquire")),(0,i.mdx)("p",null,"Same as a regular item but everything is synchronized via parent item."),(0,i.mdx)("p",null,(0,i.mdx)("strong",{parentName:"p"},"Moving racing against Acquire")),(0,i.mdx)("p",null,"Same as a regular item but everything is synchronized via parent item."),(0,i.mdx)("p",null,(0,i.mdx)("strong",{parentName:"p"},"Eviction racing against Moving")),(0,i.mdx)("p",null,"Same as a regular item but everything is synchronized via parent item."),(0,i.mdx)("p",null,(0,i.mdx)("strong",{parentName:"p"},"Eviction, Moving, Acquire all racing against each other")),(0,i.mdx)("p",null,"Same as a regular item but everything is synchronized via parent item."),(0,i.mdx)("p",null,(0,i.mdx)("strong",{parentName:"p"},"Transfer-Ownership racing against Eviction")),(0,i.mdx)("ol",null,(0,i.mdx)("li",{parentName:"ol"},"Thread-A: user or an internal thread attempting to transfer the chain to a new copy of Foobar"),(0,i.mdx)("li",{parentName:"ol"},"Thread-B: user attempting to evict Foobar to make space for another allocation"),(0,i.mdx)("li",{parentName:"ol"},"{Thread-A | Thread-B} wins",(0,i.mdx)("ol",{parentName:"li"},(0,i.mdx)("li",{parentName:"ol"},"Thread-A wins.",(0,i.mdx)("ol",{parentName:"li"},(0,i.mdx)("li",{parentName:"ol"},"Thread-A has the handle to old Foobar and has not yet transferred the chain. Thread-B cannot mark old Foobar as exclusive and will skip it to find another item to evict."),(0,i.mdx)("li",{parentName:"ol"},"Thread-A has already transferred the chain and dropped the old Foobar handle. Thread B will not come across old copy of Foobar as it will have been freed already."))),(0,i.mdx)("li",{parentName:"ol"},"Thread-B wins. This can only happen if Thread-A has not yet looked up Foobar. After eviction, Thread-A will get a miss.")))),(0,i.mdx)("p",null,(0,i.mdx)("strong",{parentName:"p"},"Transfer-Ownership racing against Moving")),(0,i.mdx)("ol",null,(0,i.mdx)("li",{parentName:"ol"},"Thread-A: user or an internal thread attempting to transfer the chain to a new copy of Foobar"),(0,i.mdx)("li",{parentName:"ol"},"Thread-B: cachelib internal thread attempting to move Foobar to another memory location"),(0,i.mdx)("li",{parentName:"ol"},"{Thread-A | Thread-B} wins",(0,i.mdx)("ol",{parentName:"li"},(0,i.mdx)("li",{parentName:"ol"},"Thread-A wins.",(0,i.mdx)("ol",{parentName:"li"},(0,i.mdx)("li",{parentName:"ol"},"Thread-A has the handle to old Foobar and has not yet transferred the chain. Thread-B cannot mark old Foobar as moving and will retry until the chain is transferred. After that it will be the same as the case below."),(0,i.mdx)("li",{parentName:"ol"},"Thread-A has already transferred the chain and dropped the old Foobar handle. Thread B will call markMovingForSlabRelease and realize the old copy of Foobar has already been freed. So it will proceed to skip to the next item to move in the slab."))),(0,i.mdx)("li",{parentName:"ol"},"Thread-B wins. This can only happen if Thread-A has not yet looked up Foobar. After moving, Thread-A will look up and get the newest copy and proceed the rest in the normal case.")))),(0,i.mdx)("p",null,(0,i.mdx)("strong",{parentName:"p"},"Chain-Add/Remove/Replace racing against Eviction")),(0,i.mdx)("ol",null,(0,i.mdx)("li",{parentName:"ol"},"Thread-A: user or an internal thread add/remove/replace a chained item"),(0,i.mdx)("li",{parentName:"ol"},"Thread-B: user attempting to evict Foobar to make space for another allocation"),(0,i.mdx)("li",{parentName:"ol"},"{Thread-A | Thread-B} wins",(0,i.mdx)("ol",{parentName:"li"},(0,i.mdx)("li",{parentName:"ol"},"Thread-A wins.",(0,i.mdx)("ol",{parentName:"li"},(0,i.mdx)("li",{parentName:"ol"},"Thread-A has the handle of Foobar and has not done anything to the chain yet. Thread-B cannot mark old Foobar as exclusive and will skip it to find another item to evict."),(0,i.mdx)("li",{parentName:"ol"},"Thread-A has finished operating on the chain and has dropped the handle of Foobar. Thread B will evict Foobar and its chained items."))),(0,i.mdx)("li",{parentName:"ol"},"Thread-B wins. This can only happen if Thread-A has not yet looked up Foobar. After eviction, Thread-A will get a miss.")))),(0,i.mdx)("p",null,(0,i.mdx)("strong",{parentName:"p"},"Chain-Add/Remove/Replace racing against Moving")),(0,i.mdx)("ol",null,(0,i.mdx)("li",{parentName:"ol"},"Thread-A: user or an internal thread add/remove/replace a chained item"),(0,i.mdx)("li",{parentName:"ol"},"Thread-B: cachelib internal thread attempting to move Foobar to another memory location"),(0,i.mdx)("li",{parentName:"ol"},"{Thread-A | Thread-B} wins",(0,i.mdx)("ol",{parentName:"li"},(0,i.mdx)("li",{parentName:"ol"},"Thread-A wins.",(0,i.mdx)("ol",{parentName:"li"},(0,i.mdx)("li",{parentName:"ol"},"Thread-A has the handle of Foobar and has not done anything to the chain yet. Thread-B cannot mark old Foobar as moving and will retry until Thread-A has completed."),(0,i.mdx)("li",{parentName:"ol"},"Thread-A has finished operating on the chain and has dropped the handle of Foobar. Thread B will move Foobar and transfer its chained items to the new parent."))),(0,i.mdx)("li",{parentName:"ol"},"Thread-B wins. This can only happen if Thread-A has not yet looked up Foobar. After moving, Thread-A will look up and get the newest copy of the parent and proceed the rest in the normal case.")))))}p.isMDXComponent=!0},80319:function(e,a,n){a.Z=n.p+"assets/images/synchronization_in_eviction_and_slab_rebalancing_chained_item_1-8c7d020564d41792ade1d96b76ac35f5.png"},83925:function(e,a,n){a.Z=n.p+"assets/images/synchronization_in_eviction_and_slab_rebalancing_chained_item_2-3ac5ac97b16f7461ed5149763e4d1b61.png"},9827:function(e,a,n){a.Z=n.p+"assets/images/synchronization_in_eviction_and_slab_rebalancing_regular_item-9bfb28eb51bf5b9b6d7e7e24991aa78c.png"}}]);