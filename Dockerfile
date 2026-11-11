# Consistent with the official Wii U plugin example build environment.
FROM ghcr.io/wiiu-env/devkitppc:20260225
COPY --from=ghcr.io/wiiu-env/wiiupluginsystem:20260418 /artifacts $DEVKITPRO
WORKDIR /project
