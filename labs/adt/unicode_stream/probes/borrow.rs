fn main() {
    let mut bytes = [0xe2u8, 0x82, 0xac];
    #[cfg(owned)] {
        let text = std::str::from_utf8(&bytes).unwrap().to_owned();
        bytes[0] = 0xff;
        assert_eq!(text, "\u{20ac}");
        assert!(std::str::from_utf8(&bytes).is_err());
    }
    #[cfg(not(owned))] {
        let text = std::str::from_utf8(&bytes).unwrap();
        #[cfg(wrong)] { bytes[0] = 0xff; }
        assert_eq!(text, "\u{20ac}");
        #[cfg(not(wrong))] {
            bytes[0] = 0xff;
            assert!(std::str::from_utf8(&bytes).is_err());
        }
    }
}
