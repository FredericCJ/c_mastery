"""Publish the fifth-edition handbook: the v4 publisher bound to the v5 catalog and manifest."""
import publish_v4


def prepare():
    return publish_v4.prepare(edition=5)


def main():
    publish_v4.main(edition=5)


if __name__ == '__main__':
    main()
