trait CopyView {
    fn length(&self) -> usize;
    fn copy(&self, out: &mut [u8]) -> usize;
}
struct Bytes([u8; 3]);
impl CopyView for Bytes {
    #[cfg(not(wrong_signature))]
    fn length(&self) -> usize { 1 }
    #[cfg(wrong_signature)]
    fn length(&self) -> bool { true }
    #[cfg(not(missing_method))]
    fn copy(&self, out: &mut [u8]) -> usize {
        out[..3].copy_from_slice(&self.0);
        3
    }
}
mod extension {
    pub struct Item(pub usize);
    pub trait Measure { fn measure(&self) -> usize; }
    impl Measure for Item { fn measure(&self) -> usize { self.0 } }
}
#[cfg(not(no_trait_import))]
use extension::Measure;
#[cfg(escaping_borrow)]
fn escape() -> &'static dyn CopyView {
    let local = Bytes([1, 0, 2]);
    &local
}
fn borrow(value: &Bytes) -> &dyn CopyView { value }
struct Marker;
fn needs_fn<F: Fn() -> usize>(f: F) { assert_eq!(f(), 3); assert_eq!(f(), 3); }
fn main() {
    let bytes = Bytes([1, 0, 2]);
    let mut out = [0; 3];
    let view = borrow(&bytes);
    let actual = view.copy(&mut out);
    assert_ne!(view.length(), actual); // Type checking did not prove the law.
    assert_eq!(extension::Item(17).measure(), 17);
    let text = String::from("abc");
    needs_fn(move || text.len()); // move capture can still implement Fn.
    #[cfg(once_twice)]
    {
        let owned = String::from("owned");
        let consume = move || drop(owned);
        consume();
        consume();
    }
    println!("length={} copy={} marker={} trait_ref={}", view.length(), actual,
             std::mem::size_of::<Marker>(), std::mem::size_of::<&dyn CopyView>());
}
