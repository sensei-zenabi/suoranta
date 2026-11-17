import test from 'node:test';
import assert from 'node:assert/strict';
import {setupClickCounters} from '../click-counters.mjs';

function catalog() {
  const listeners = new Map();
  const button = {href:'download.zip',addEventListener(type,fn){listeners.set(type,fn);}};
  const stats=[];
  const row = {querySelector(){return button;},append(node){stats.push(node);}};
  return {listeners,button,stats,root:{querySelectorAll(){return [{dataset:{release:'write'},closest(){return row;}}];},createElement(){return {};}}};
}
const response = value => ({ok:true,json:async()=>value});
const settle = () => new Promise(resolve=>setImmediate(resolve));
test('page views read zero without increments; mouse, keyboard-style and middle clicks record once',async()=>{
  const c=catalog(), calls=[];
  await setupClickCounters(c.root, async (url,options)=>{calls.push({url,options});return response(options.method ? {count:calls.filter(c=>c.options.method).length} : {counts:{write:0}});});
  assert.equal(c.stats[0].textContent,'0 clicks');
  assert.equal(calls.length,1);
  c.listeners.get('click')({isTrusted:true,type:'click',button:0});
  await settle();
  assert.equal(c.stats[0].textContent,'1 click');
  c.listeners.get('auxclick')({isTrusted:true,type:'auxclick',button:1});
  await settle();
  assert.equal(c.stats[0].textContent,'2 clicks');
  c.listeners.get('auxclick')({isTrusted:true,type:'auxclick',button:2});
  c.listeners.get('click')({isTrusted:false,type:'click',button:0});
  await settle();
  assert.equal(calls.length,3);
  assert.equal(calls[1].options.keepalive,true);
  assert.notEqual(calls[1].options.body,calls[2].options.body);
  assert.equal(c.button.href,'download.zip');
});
test('network retry reuses ID; failure preserves browser download behavior',async()=>{
  const c=catalog(), posts=[];
  await setupClickCounters(c.root,async(url,options)=>{
    if(!options.method)return response({counts:{write:0}});
    posts.push(options);
    throw Error('offline');
  });
  c.listeners.get('click')({isTrusted:true,type:'click',button:0});
  await settle();
  assert.equal(posts.length,2);
  assert.equal(posts[0].body,posts[1].body);
  assert.equal(c.button.href,'download.zip');
  assert.equal(c.stats[0].textContent,'Clicks unavailable');
});
test('slow initial read cannot overwrite a newer click total',async()=>{
  const c=catalog();let resolveRead;
  const ready=setupClickCounters(c.root,async(url,options)=>options.method ? response({count:1}) : new Promise(resolve=>{resolveRead=resolve;}));
  c.listeners.get('click')({isTrusted:true,type:'click',button:0});await settle();
  resolveRead(response({counts:{write:0}}));await ready;
  assert.equal(c.stats[0].textContent,'1 click');
});
