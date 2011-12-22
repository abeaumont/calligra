#include "SvgCustomSavingContext.h"
#include <KoXmlWriter.h>
#include <QDebug>
#include <qfile.h>

SvgCustomSavingContext::SvgCustomSavingContext()
{
    m_animationPropertiesWriter = new KoXmlWriter(&m_animationPropertiesBuffer, 1);
    saveScript();
}

bool SvgCustomSavingContext::saveScript()
{
    /*QFile sozi("sozi.js");//TODO: What path to set for file?
    if(!sozi.open(QIODevice::ReadOnly)){
      return false;
    }
     QTextStream in(&sozi);
     QString s = in.readAll();//TODO: A better way to do these 2 steps together?
     */
     QString s("var sozi=sozi||{};(function(){var a=sozi.events=sozi.events||{},b={};a.listen=function(d,e){var c=b[d];if(!c){c=b[d]=[]}c.push(e)};a.fire=function(g){var d=b[g],c,f,e=Array.prototype.slice.call(arguments,1);if(d){c=d.length;for(f=0;f&lt;c;f+=1){d[f].apply(null,e)}}}}());var sozi=sozi||{};(function(){var w=sozi.framelist=sozi.framelist||{},k=this,r=k.document,q,m,c,j=0,d=5,i,u,b,y,z,v,f=300,g=&quot;decelerate&quot;,a=&quot;http://www.w3.org/2000/svg&quot;;function h(A){return function(B){sozi.player.previewFrame(A);B.stopPropagation()}}function l(A){A.stopPropagation()}function p(B){var A=B.relatedTarget;while(A!==m&amp;&amp;A!==q){A=A.parentNode}if(A===q){w.hide();sozi.player.restart();B.stopPropagation()}}function t(B){var A=c.getCTM().f;if(A&lt;=-k.innerHeight/2){A+=k.innerHeight/2}else{if(A&lt;0){A=0}}c.setAttribute(&quot;transform&quot;,&quot;translate(0,&quot;+A+&quot;)&quot;);B.stopPropagation()}function e(B){var A=c.getCTM().f;if(A+j&gt;=k.innerHeight*3/2){A-=k.innerHeight/2}else{if(A+j&gt;k.innerHeight+2*d){A=k.innerHeight-j-4*d}}c.setAttribute(&quot;transform&quot;,&quot;translate(0,&quot;+A+&quot;)&quot;);B.stopPropagation()}function s(A){var C=sozi.animation.profiles[g](A),B=1-C;z=y*C+b*B;m.setAttribute(&quot;transform&quot;,&quot;translate(&quot;+z+&quot;,0)&quot;)}function x(){}function o(){var G=r.createElementNS(a,&quot;rect&quot;),A=r.createElementNS(a,&quot;path&quot;),E=r.createElementNS(a,&quot;path&quot;),H=0,F,C=sozi.document.frames.length,B=sozi.location.getFrameIndex(),D,I;q=r.documentElement;m=r.createElementNS(a,&quot;g&quot;);m.setAttribute(&quot;id&quot;,&quot;sozi-toc&quot;);q.appendChild(m);c=r.createElementNS(a,&quot;g&quot;);m.appendChild(c);G.setAttribute(&quot;id&quot;,&quot;sozi-toc-background&quot;);G.setAttribute(&quot;x&quot;,d);G.setAttribute(&quot;y&quot;,d);G.setAttribute(&quot;rx&quot;,d);G.setAttribute(&quot;ry&quot;,d);G.addEventListener(&quot;click&quot;,l,false);G.addEventListener(&quot;mousedown&quot;,l,false);G.addEventListener(&quot;mouseout&quot;,p,false);c.appendChild(G);for(D=0;D&lt;C;D+=1){I=r.createElementNS(a,&quot;text&quot;);I.appendChild(r.createTextNode(sozi.document.frames[D].title));c.appendChild(I);if(D===B){I.setAttribute(&quot;class&quot;,&quot;sozi-toc-current&quot;)}F=I.getBBox().width;j+=I.getBBox().height;if(F&gt;H){H=F}I.setAttribute(&quot;x&quot;,2*d);I.setAttribute(&quot;y&quot;,j+d);I.addEventListener(&quot;click&quot;,h(D),false);I.addEventListener(&quot;mousedown&quot;,l,false)}A.setAttribute(&quot;class&quot;,&quot;sozi-toc-arrow&quot;);A.setAttribute(&quot;d&quot;,&quot;M&quot;+(H+3*d)+&quot;,&quot;+(5*d)+&quot; l&quot;+(4*d)+&quot;,0 l-&quot;+(2*d)+&quot;,-&quot;+(3*d)+&quot; z&quot;);A.addEventListener(&quot;click&quot;,t,false);A.addEventListener(&quot;mousedown&quot;,l,false);m.appendChild(A);E.setAttribute(&quot;class&quot;,&quot;sozi-toc-arrow&quot;);E.setAttribute(&quot;d&quot;,&quot;M&quot;+(H+3*d)+&quot;,&quot;+(7*d)+&quot; l&quot;+(4*d)+&quot;,0 l-&quot;+(2*d)+&quot;,&quot;+(3*d)+&quot; z&quot;);E.addEventListener(&quot;click&quot;,e,false);E.addEventListener(&quot;mousedown&quot;,l,false);m.appendChild(E);G.setAttribute(&quot;width&quot;,H+7*d);G.setAttribute(&quot;height&quot;,j+2*d);i=-H-9*d;u=0;z=y=i;m.setAttribute(&quot;transform&quot;,&quot;translate(&quot;+i+&quot;,0)&quot;);v=new sozi.animation.Animator(s,x)}function n(A){var D=r.getElementsByClassName(&quot;sozi-toc-current&quot;),C=c.getElementsByTagName(&quot;text&quot;),B;for(B=0;B&lt;D.length;B+=1){D[B].removeAttribute(&quot;class&quot;)}C[A].setAttribute(&quot;class&quot;,&quot;sozi-toc-current&quot;)}w.show=function(){b=z;y=u;v.start(f)};w.hide=function(){b=z;y=i;v.start(f)};w.isVisible=function(){return y===u};sozi.events.listen(&quot;displayready&quot;,o);sozi.events.listen(&quot;cleanup&quot;,w.hide);sozi.events.listen(&quot;framechange&quot;,n)}());var sozi=sozi||{};(function(){var t=sozi.player=sozi.player||{},u=sozi.display=sozi.display||{},j=this,o=j.document,p=0,v=1,s=1.05,x=5,c=false,l=false,g=0,e=0;function a(A,z,B){t.stop();u.zoom(A&gt;0?s:1/s,z,B)}function w(y){t.stop();u.rotate(y&gt;0?x:-x)}function r(){if(sozi.framelist.isVisible()){sozi.framelist.hide();t.restart()}else{t.stop();sozi.framelist.show()}}function i(y){if(y.button===p){c=true;l=false;g=y.clientX;e=y.clientY}else{if(y.button===v){r()}}y.stopPropagation()}function m(y){if(c){t.stop();l=true;sozi.events.fire(&quot;cleanup&quot;);u.drag(y.clientX-g,y.clientY-e);g=y.clientX;e=y.clientY}y.stopPropagation()}function f(y){if(y.button===p){c=false}y.stopPropagation()}function q(y){t.moveToPrevious();y.stopPropagation();y.preventDefault()}function h(y){if(!l&amp;&amp;y.button!==v){t.moveToNext()}y.stopPropagation()}function k(y){var z=0;if(!y){y=j.event}if(y.wheelDelta){z=y.wheelDelta;if(j.opera){z=-z}}else{if(y.detail){z=-y.detail}}if(z!==0){if(y.shiftKey){w(z)}else{a(z,y.clientX,y.clientY)}}y.stopPropagation();y.preventDefault()}function n(y){switch(y.charCode){case 43:a(1,j.innerWidth/2,j.innerHeight/2);break;case 45:a(-1,j.innerWidth/2,j.innerHeight/2);break;case 61:t.moveToCurrent();break;case 70:case 102:t.showAll();break;case 84:case 116:r();break;case 82:w(-1);break;case 114:w(1);break}y.stopPropagation()}function d(y){switch(y.keyCode){case 36:t.moveToFirst();break;case 35:t.moveToLast();break;case 38:t.jumpToPrevious();break;case 33:case 37:t.moveToPrevious();break;case 40:t.jumpToNext();break;case 34:case 39:case 13:case 32:t.moveToNext();break}y.stopPropagation()}function b(){var y=o.documentElement;y.addEventListener(&quot;click&quot;,h,false);y.addEventListener(&quot;mousedown&quot;,i,false);y.addEventListener(&quot;mouseup&quot;,f,false);y.addEventListener(&quot;mousemove&quot;,m,false);y.addEventListener(&quot;keypress&quot;,n,false);y.addEventListener(&quot;keydown&quot;,d,false);y.addEventListener(&quot;contextmenu&quot;,q,false);y.addEventListener(&quot;DOMMouseScroll&quot;,k,false);j.onmousewheel=k}j.addEventListener(&quot;load&quot;,b,false)}());var sozi=sozi||{};(function(){var e=sozi.animation=sozi.animation||{},h=this,j=40,d=[],b,i=h.mozRequestAnimationFrame||h.webkitRequestAnimationFrame||h.msRequestAnimationFrame||h.oRequestAnimationFrame;function c(){if(i){i(f)}else{b=h.setInterval(function(){f(Date.now())},j)}}function f(l){var k;if(d.length&gt;0){if(i){i(f)}for(k=0;k&lt;d.length;k+=1){d[k].step(l)}}else{if(!i){h.clearInterval(b)}}}function a(k){d.push(k);if(d.length===1){c()}}function g(k){d.splice(d.indexOf(k),1)}e.Animator=function(k,l){this.onStep=k;this.onDone=l;this.durationMs=0;this.data={};this.initialTime=0;this.started=false};e.Animator.prototype.start=function(k,l){this.durationMs=k;this.data=l;this.initialTime=Date.now();this.onStep(0,this.data);if(!this.started){this.started=true;a(this)}};e.Animator.prototype.stop=function(){if(this.started){g(this);this.started=false}};e.Animator.prototype.step=function(l){var k=l-this.initialTime;if(k&gt;=this.durationMs){this.stop();this.onStep(1,this.data);this.onDone()}else{this.onStep(k/this.durationMs,this.data)}};e.profiles={linear:function(k){return k},accelerate:function(k){return Math.pow(k,3)},&quot;strong-accelerate&quot;:function(k){return Math.pow(k,5)},decelerate:function(k){return 1-Math.pow(1-k,3)},&quot;strong-decelerate&quot;:function(k){return 1-Math.pow(1-k,5)},&quot;accelerate-decelerate&quot;:function(k){var l=k&lt;=0.5?k:1-k,m=Math.pow(2*l,3)/2;return k&lt;=0.5?m:1-m},&quot;strong-accelerate-decelerate&quot;:function(k){var l=k&lt;=0.5?k:1-k,m=Math.pow(2*l,5)/2;return k&lt;=0.5?m:1-m},&quot;decelerate-accelerate&quot;:function(k){var l=k&lt;=0.5?k:1-k,m=(1-Math.pow(1-2*l,2))/2;return k&lt;=0.5?m:1-m},&quot;strong-decelerate-accelerate&quot;:function(k){var l=k&lt;=0.5?k:1-k,m=(1-Math.pow(1-2*l,3))/2;return k&lt;=0.5?m:1-m}}}());var sozi=sozi||{};(function(){var d=sozi.player=sozi.player||{},j=sozi.display=sozi.display||{},h=this,c,k,e=500,q=-10,l=&quot;linear&quot;,g=0,n=0,o=false,p=false;function f(t,w){var v=1-t,u=w.profile(t),s=1-u,r,x;for(r in w.initialState){if(w.initialState.hasOwnProperty(r)){if(typeof w.initialState[r]===&quot;number&quot;&amp;&amp;typeof w.finalState[r]===&quot;number&quot;){j.geometry[r]=w.finalState[r]*u+w.initialState[r]*s}}}if(w.zoomWidth&amp;&amp;w.zoomWidth.k!==0){x=t-w.zoomWidth.ts;j.geometry.width=w.zoomWidth.k*x*x+w.zoomWidth.ss}if(w.zoomHeight&amp;&amp;w.zoomHeight.k!==0){x=t-w.zoomHeight.ts;j.geometry.height=w.zoomHeight.k*x*x+w.zoomHeight.ss}j.clip=w.finalState.clip;j.update()}function i(){var r;if(sozi.document.frames[n].timeoutEnable){p=true;r=(n+1)%sozi.document.frames.length;k=h.setTimeout(function(){d.moveToFrame(r)},sozi.document.frames[n].timeoutMs)}}function m(){g=n;if(o){i()}}d.startFromIndex=function(r){o=true;p=false;g=r;n=r;j.showFrame(sozi.document.frames[r]);i()};d.restart=function(){d.startFromIndex(n)};d.stop=function(){c.stop();if(p){h.clearTimeout(k);p=false}o=false;g=n};function b(r,B,z){var C={ss:((r&lt;0)?Math.max(B,z):Math.min(B,z))*(100-r)/100,ts:0.5,k:0},x,w,t,s,A,y;if(r!==0){x=B-z;w=B-C.ss;t=z-C.ss;if(x!==0){s=Math.sqrt(w*t);A=(w-s)/x;y=(w+s)/x;C.ts=(A&gt;0&amp;&amp;A&lt;=1)?A:y}C.k=w/C.ts/C.ts}return C}d.jumpToFrame=function(r){d.stop();sozi.events.fire(&quot;cleanup&quot;);g=r;n=r;j.showFrame(sozi.document.frames[r]);sozi.events.fire(&quot;framechange&quot;,r)};d.previewFrame=function(s){var u=sozi.document.frames[s].geometry,r,t;if(q!==0){r=b(q,j.geometry.width,u.width);t=b(q,j.geometry.height,u.height)}n=s;c.start(e,{initialState:j.getCurrentGeometry(),finalState:u,profile:sozi.animation.profiles[l],zoomWidth:r,zoomHeight:t});sozi.events.fire(&quot;framechange&quot;,s)};d.moveToFrame=function(t){var s=e,w=q,u=sozi.animation.profiles[l],r,v;if(p){h.clearTimeout(k);p=false}if(t===(n+1)%sozi.document.frames.length){s=sozi.document.frames[t].transitionDurationMs;w=sozi.document.frames[t].transitionZoomPercent;u=sozi.document.frames[t].transitionProfile}sozi.events.fire(&quot;cleanup&quot;);if(w!==0){r=b(w,j.geometry.width,sozi.document.frames[t].geometry.width);v=b(w,j.geometry.height,sozi.document.frames[t].geometry.height)}o=true;n=t;c.start(s,{initialState:j.getCurrentGeometry(),finalState:sozi.document.frames[n].geometry,profile:u,zoomWidth:r,zoomHeight:v});sozi.events.fire(&quot;framechange&quot;,t)};d.moveToFirst=function(){d.moveToFrame(0)};d.jumpToPrevious=function(){var r=n;if(!c.started||g&lt;=n){r-=1}if(r&gt;=0){d.jumpToFrame(r)}};d.moveToPrevious=function(){var r=n,s;for(r-=1;r&gt;=0;r-=1){s=sozi.document.frames[r];if(!s.timeoutEnable||s.timeoutMs!==0){d.moveToFrame(r);break}}};d.jumpToNext=function(){var r=n;if(!c.started||g&gt;=n){r+=1}if(r&lt;sozi.document.frames.length){d.jumpToFrame(r)}};d.moveToNext=function(){if(n&lt;sozi.document.frames.length-1||sozi.document.frames[n].timeoutEnable){d.moveToFrame((n+1)%sozi.document.frames.length)}};d.moveToLast=function(){d.moveToFrame(sozi.document.frames.length-1)};d.moveToCurrent=function(){d.moveToFrame(n)};d.showAll=function(){d.stop();sozi.events.fire(&quot;cleanup&quot;);c.start(e,{initialState:j.getCurrentGeometry(),finalState:j.getDocumentGeometry(),profile:sozi.animation.profiles[l]})};function a(){d.startFromIndex(sozi.location.getFrameIndex())}c=new sozi.animation.Animator(f,m);sozi.events.listen(&quot;displayready&quot;,a)}());var sozi=sozi||{};(function(){var d=sozi.display=sozi.display||{},h=this,i=h.document,j,c,k,a,g=&quot;http://www.w3.org/2000/svg&quot;;d.geometry={cx:0,cy:0,width:1,height:1,rotate:0,clip:true};function f(){var o,l=i.createElementNS(g,&quot;g&quot;),m=i.createElementNS(g,&quot;clipPath&quot;);j=i.documentElement;k=j.getBBox();a=i.createElementNS(g,&quot;g&quot;);while(true){o=j.firstChild;if(!o){break}j.removeChild(o);a.appendChild(o)}c=i.createElementNS(g,&quot;rect&quot;);c.setAttribute(&quot;id&quot;,&quot;sozi-clip-rect&quot;);m.setAttribute(&quot;id&quot;,&quot;sozi-clip-path&quot;);m.appendChild(c);j.appendChild(m);l.setAttribute(&quot;clip-path&quot;,&quot;url(#sozi-clip-path)&quot;);l.appendChild(a);j.appendChild(l);j.setAttribute(&quot;width&quot;,h.innerWidth);j.setAttribute(&quot;height&quot;,h.innerHeight);sozi.events.fire(&quot;displayready&quot;)}function b(){j.setAttribute(&quot;width&quot;,h.innerWidth);j.setAttribute(&quot;height&quot;,h.innerHeight);d.update()}function e(){var l={};l.scale=Math.min(h.innerWidth/d.geometry.width,h.innerHeight/d.geometry.height);l.width=d.geometry.width*l.scale;l.height=d.geometry.height*l.scale;l.x=(h.innerWidth-l.width)/2;l.y=(h.innerHeight-l.height)/2;return l}d.getElementGeometry=function(l){var s,p,t,n,q,o,r=l.getCTM(),m=Math.sqrt(r.a*r.a+r.b*r.b);if(l.nodeName===&quot;rect&quot;){s=l.x.baseVal.value;p=l.y.baseVal.value;t=l.width.baseVal.value;n=l.height.baseVal.value}else{q=l.getBBox();s=q.x;p=q.y;t=q.width;n=q.height}o=i.documentElement.createSVGPoint();o.x=s+t/2;o.y=p+n/2;o=o.matrixTransform(r);return{cx:o.x,cy:o.y,width:t*m,height:n*m,rotate:Math.atan2(r.b,r.a)*180/Math.PI}};d.getDocumentGeometry=function(){return{cx:k.x+k.width/2,cy:k.y+k.height/2,width:k.width,height:k.height,rotate:0,clip:false}};d.getCurrentGeometry=function(){return{cx:d.geometry.cx,cy:d.geometry.cy,width:d.geometry.width,height:d.geometry.height,rotate:d.geometry.rotate,clip:d.geometry.clip}};d.update=function(){var l=e(),n=-d.geometry.cx+d.geometry.width/2+l.x/l.scale,m=-d.geometry.cy+d.geometry.height/2+l.y/l.scale;a.setAttribute(&quot;transform&quot;,&quot;scale(&quot;+l.scale+&quot;)translate(&quot;+n+&quot;,&quot;+m+&quot;)rotate(&quot;+(-d.geometry.rotate)+&quot;,&quot;+d.geometry.cx+&quot;,&quot;+d.geometry.cy+&quot;)&quot;);c.setAttribute(&quot;x&quot;,d.geometry.clip?l.x:0);c.setAttribute(&quot;y&quot;,d.geometry.clip?l.y:0);c.setAttribute(&quot;width&quot;,d.geometry.clip?l.width:h.innerWidth);c.setAttribute(&quot;height&quot;,d.geometry.clip?l.height:h.innerHeight)};d.showFrame=function(m){var l;for(l in m.geometry){if(m.geometry.hasOwnProperty(l)){d.geometry[l]=m.geometry[l]}}d.update()};d.drag=function(m,l){var n=e(),o=d.geometry.rotate*Math.PI/180;d.geometry.cx-=(m*Math.cos(o)-l*Math.sin(o))/n.scale;d.geometry.cy-=(m*Math.sin(o)+l*Math.cos(o))/n.scale;d.geometry.clip=false;d.update()};d.zoom=function(o,m,p){var n=(1-o)*(m-h.innerWidth/2),l=(1-o)*(p-h.innerHeight/2);d.geometry.width/=o;d.geometry.height/=o;d.drag(n,l)};d.rotate=function(l){d.geometry.rotate+=l;d.geometry.rotate%=360;d.update()};sozi.events.listen(&quot;documentready&quot;,f);h.addEventListener(&quot;resize&quot;,b,false)}());var sozi=sozi||{};(function(){var c=sozi.document=sozi.document||{},g=this,a=g.document,f=&quot;http://sozi.baierouge.fr&quot;,d={title:&quot;Untitled&quot;,sequence:&quot;0&quot;,hide:&quot;true&quot;,clip:&quot;true&quot;,&quot;timeout-enable&quot;:&quot;false&quot;,&quot;timeout-ms&quot;:&quot;5000&quot;,&quot;transition-duration-ms&quot;:&quot;1000&quot;,&quot;transition-zoom-percent&quot;:&quot;0&quot;,&quot;transition-profile&quot;:&quot;linear&quot;};c.frames=[];function h(j,i){var k=j.getAttributeNS(f,i);return k===&quot;&quot;?d[i]:k}function b(){var j=a.getElementsByTagNameNS(f,&quot;frame&quot;),k=j.length,m,l,n;for(l=0;l&lt;k;l+=1){m=a.getElementById(j[l].getAttributeNS(f,&quot;refid&quot;));if(m){n={geometry:sozi.display.getElementGeometry(m),title:h(j[l],&quot;title&quot;),sequence:parseInt(h(j[l],&quot;sequence&quot;),10),hide:h(j[l],&quot;hide&quot;)===&quot;true&quot;,timeoutEnable:h(j[l],&quot;timeout-enable&quot;)===&quot;true&quot;,timeoutMs:parseInt(h(j[l],&quot;timeout-ms&quot;),10),transitionDurationMs:parseInt(h(j[l],&quot;transition-duration-ms&quot;),10),transitionZoomPercent:parseInt(h(j[l],&quot;transition-zoom-percent&quot;),10),transitionProfile:sozi.animation.profiles[h(j[l],&quot;transition-profile&quot;)||&quot;linear&quot;]};if(n.hide){m.setAttribute(&quot;visibility&quot;,&quot;hidden&quot;)}n.geometry.clip=h(j[l],&quot;clip&quot;)===&quot;true&quot;;c.frames.push(n)}}c.frames.sort(function(o,i){return o.sequence-i.sequence})}function e(){a.documentElement.removeAttribute(&quot;viewBox&quot;);b();sozi.events.fire(&quot;documentready&quot;)}g.addEventListener(&quot;load&quot;,e,false)}());var sozi=sozi||{};(function(){var a=sozi.location=sozi.location||{},e=this,c=false;a.getFrameIndex=function(){var g=e.location.hash?parseInt(e.location.hash.slice(1),10)-1:0;if(isNaN(g)||g&lt;0){return 0}else{if(g&gt;=sozi.document.frames.length){return sozi.document.frames.length-1}else{return g}}};function f(){var g=a.getFrameIndex();if(!c){sozi.player.moveToFrame(g)}c=false}function d(g){c=true;e.location.hash=&quot;#&quot;+(g+1)}function b(){sozi.events.listen(&quot;framechange&quot;,d)}e.addEventListener(&quot;hashchange&quot;,f,false);e.addEventListener(&quot;load&quot;,b,false)}());");
     
     m_animationPropertiesWriter->startElement("script");
     m_animationPropertiesWriter->addAttribute("id", "sozi-script");
     m_animationPropertiesWriter->addTextNode(s);
     m_animationPropertiesWriter->endElement();
     
     return true;
}

SvgCustomSavingContext::~SvgCustomSavingContext()
{
   
}

KoXmlWriter& SvgCustomSavingContext::animationPropertiesWriter()
{
    return *m_animationPropertiesWriter;
}

bool SvgCustomSavingContext::finalize()
{
    SvgSavingContext::finalize();
    outputDevice().write("\n");
    outputDevice().write(m_animationPropertiesBuffer.data());
    outputDevice().write("\n");
    
    return true;
}

