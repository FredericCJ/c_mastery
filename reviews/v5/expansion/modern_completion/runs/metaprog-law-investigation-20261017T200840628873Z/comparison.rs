use std::hash::{Hash,Hasher,DefaultHasher};
#[derive(PartialEq,Eq,Hash)] struct Derived { text:Vec<u8>,cache:u32 }
struct Semantic { text:Vec<u8>,cache:u32 }
fn norm(x:&[u8])->Vec<u8>{x.split(|b|*b==b' ').filter(|s|!s.is_empty()).map(|s|s.iter().map(u8::to_ascii_lowercase).collect::<Vec<_>>()).collect::<Vec<_>>().join(&b' ')}
impl PartialEq for Semantic {fn eq(&self,b:&Self)->bool{norm(&self.text)==norm(&b.text)}}
impl Eq for Semantic {}
impl Hash for Semantic {fn hash<H:Hasher>(&self,h:&mut H){norm(&self.text).hash(h)}}
fn hashed<T:Hash>(x:&T)->u64{let mut h=DefaultHasher::new();x.hash(&mut h);h.finish()}
fn main(){let a=Semantic{text:b" A  B ".to_vec(),cache:1};let b=Semantic{text:b"a b".to_vec(),cache:99};assert!(a==b);assert_eq!(hashed(&a),hashed(&b));assert_ne!(a.cache,b.cache);let x=Derived{text:b"a".to_vec(),cache:1};let y=Derived{text:b"a".to_vec(),cache:2};assert!(x!=y);println!("MPT_RUST derived-cache-semantic-difference custom-canonical-eq-hash no-stable-hash-claim");}
