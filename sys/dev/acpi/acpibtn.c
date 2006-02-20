/* $OpenBSD: acpibtn.c,v 1.1 2006/02/20 00:48:10 marco Exp $ */
/*
 * Copyright (c) 2005 Marco Peereboom <marco@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>
#include <sys/malloc.h>

#include <machine/bus.h>

#include <dev/acpi/acpireg.h>
#include <dev/acpi/acpivar.h>
#include <dev/acpi/acpidev.h>
#include <dev/acpi/amltypes.h>
#include <dev/acpi/dsdt.h>

#include <sys/sensors.h>

int acpibtn_match(struct device *, void *, void *);
void acpibtn_attach(struct device *, struct device *, void *);
int  acpibtn_notify(struct aml_node *, int, void *);

struct acpibtn_softc {
	struct device		sc_dev;

	bus_space_tag_t		sc_iot;
	bus_space_handle_t	sc_ioh;

	struct acpi_softc	*sc_acpi;
	struct aml_node		*sc_devnode;

	int			sc_pwr_btn;
	int			sc_lid_btn;
	int			sc_sleep_btn;
#if 0
	struct sensor sens[3];	/* XXX debug only */
#endif
};


#if 0
void acpibtn_refresh(void *);
#endif
int acpibtn_getsta(struct acpibtn_softc *);

struct cfattach acpibtn_ca = {
	sizeof(struct acpibtn_softc), acpibtn_match, acpibtn_attach
};

struct cfdriver acpibtn_cd = {
	NULL, "acpibtn", DV_DULL
};

int
acpibtn_match(struct device *parent, void *match, void *aux)
{
	struct acpi_attach_args *aa = aux;
	struct cfdata *cf = match;

	/* sanity */
	if (aa->aaa_name == NULL ||
	    strcmp(aa->aaa_name, cf->cf_driver->cd_name) != 0 ||
	    aa->aaa_table != NULL)
		return (0);

	return (1);
}

void
acpibtn_attach(struct device *parent, struct device *self, void *aux)
{
	struct acpibtn_softc *sc = (struct acpibtn_softc *)self;
	struct acpi_attach_args *aa = aux;

	sc->sc_acpi = (struct acpi_softc *)parent;
	sc->sc_devnode = aa->aaa_node->child;

	acpibtn_getsta(sc); 

	/* XXX print which buttons are available and state */
	printf("\n");

	aml_register_notify(sc->sc_devnode->parent, acpibtn_notify, sc);

	/* XXX: fixme */
	sc->sc_acpi->sc_pbtndev = sc->sc_devnode;
	sc->sc_acpi->sc_sbtndev = sc->sc_devnode;
#if 0
	strlcpy(sc->sens[0].device, DEVNAME(sc), sizeof(sc->sens[0].device));
	strlcpy(sc->sens[0].desc, "power supply", sizeof(sc->sens[2].desc));
	sc->sens[0].type = SENSOR_INDICATOR;
	sensor_add(&sc->sens[0]);
	sc->sens[0].value = sc->sc_ac_stat;

	if (sensor_task_register(sc, acpibtn_refresh, 10))
		printf(", unable to register update task\n");
#endif
}

#if 0
/* XXX this is for debug only, remove later */
void
acpibtn_refresh(void *arg)
{
	struct acpibtn_softc *sc = arg;

	acpibtn_getsta(sc); 

	sc->sens[0].value = sc->sc_ac_stat;
}
#endif

int
acpibtn_getsta(struct acpibtn_softc *sc)
{
	struct aml_value res, env;
	struct acpi_context *ctx;

	memset(&res, 0, sizeof(res));
	memset(&env, 0, sizeof(env));

	ctx = NULL;
	if (aml_eval_name(sc->sc_acpi, sc->sc_devnode, "_STA", &res, &env)) {
		dnprintf(10, "%s: no _STA\n",
		    DEVNAME(sc));
		/* XXX this should fall through */
		return (1);
	}

	return (0);
}

int
acpibtn_notify(struct aml_node *node, int notify_type, void *arg)
{
	struct acpibtn_softc *sc = arg;

	printf("acpibtn_notify: %.2x %s\n", notify_type, sc->sc_devnode->name);
	return (0);
}
